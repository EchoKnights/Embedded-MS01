// // server_win_pipe.c
// // Compile (MSVC): cl server_win_pipe.c ws2_32.lib
// // or with mingw: gcc server_win_pipe.c -o server_win_pipe -lws2_32
// #define _WIN32_WINNT 0x0600
// #include <windows.h>
// #include <stdio.h>

// #define PIPE_NAME "\\\\.\\pipe\\MyPipe"
// #define BUFSIZE 256

// int main(void) {
// 	HANDLE hPipe;
// 	char buf[BUFSIZE];
// 	DWORD bytesRead;

// 	printf("Creating named pipe %s\n", PIPE_NAME);

// 	hPipe = CreateNamedPipeA(
// 		PIPE_NAME,
// 		PIPE_ACCESS_INBOUND,       // server reads
// 		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
// 		1,                         // max instances
// 		0, 0,                      // out/in buffer sizes (0 -> default)
// 		0,                         // default timeout
// 		NULL
// 	);

// 	if (hPipe == INVALID_HANDLE_VALUE) {
// 		fprintf(stderr, "CreateNamedPipe failed: %lu\n", GetLastError());
// 		return 1;
// 	}

// 	printf("Waiting for client to connect...\n");
// 	if (!ConnectNamedPipe(hPipe, NULL)) {
// 		fprintf(stderr, "ConnectNamedPipe failed: %lu\n", GetLastError());
// 		CloseHandle(hPipe);
// 		return 1;
// 	}

// 	if (ReadFile(hPipe, buf, BUFSIZE - 1, &bytesRead, NULL) && bytesRead > 0) {
// 		buf[bytesRead] = '\0';
// 		/* trim newline */
// 		char *nl = strchr(buf, '\n');
// 		if (nl) *nl = '\0';
// 		printf("Received: '%s'\n", buf);
// 	} else {
// 		fprintf(stderr, "ReadFile failed: %lu\n", GetLastError());
// 	}

// 	DisconnectNamedPipe(hPipe);
// 	CloseHandle(hPipe);
// 	return 0;
// }

// usb_hold_detector.c
// Pico SDK program: detect a logical "hold" asserted from host via USB-CDC.
// Host protocol:
//   "H\n"      -> host asserts (start of hold). Pico records timestamp.
//   "HB\n"     -> heartbeat while asserted (optional but recommended).
//   "L\n"      -> host releases (end of hold).
//
// Pico semantics:
//   - If input is continuously asserted for HOLD_MS (5000 ms), take next step.
//   - Heartbeat updates the "last seen" timestamp; if no message seen for TIMEOUT_MS, treat as released.
//   - Non-blocking: main loop stays responsive.

#include "webcam_driver.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

float poll_usb_distance(void) {
	static char line[MAX_LINE];
	static int pos = 0;

	while (true) {
		int c = getchar_timeout_us(0);
		if (c == PICO_ERROR_TIMEOUT) {
			// No more data right now
			return USB_NO_DATA;   // e.g. -1.0f
		}

		if (c == '\r') {
			continue; // ignore CR
		}

		if (c == '\n') {
			line[pos] = '\0';
			pos = 0;

			if (strlen(line) == 0) {
				return USB_NO_DATA;
			}

			// Try parsing float
			char *end;
			float value = strtof(line, &end);
			if (end == line) {
				return USB_PARSE_ERROR; // e.g. -2.0f
			}

			return value; // ✅ valid distance
		}

		if (pos < MAX_LINE - 1) {
			line[pos++] = (char)c;
		} else {
			pos = 0; // overflow, reset
			return USB_PARSE_ERROR;
		}
	}
}

// bool control_while_maintaining_assert_for(
// 	void (*motor_funct_ptr)(uint, uint, uint), 
// 	void (*keypad_funct_ptr)(uint, uint, uint, uint, uint, uint, uint),
// 	float time_seconds
// ){
// 	uint64_t begin_time = to_ms_since_boot(get_absolute_time());
// 	while(ms)
// }

int original(void) {
	printf("usb_hold_detector ready\r\n");

	char line[MAX_LINE];
	int pos = 0;

	bool asserted = false;            // logical input state as we interpret it
	uint64_t last_seen_ms = 0;        // last time we saw an H or HB message
	bool triggered = false;           // whether we've already taken the "next step" for current assertion

	while (true) {
		// non-blocking read
		int c = getchar_timeout_us(0);
		if (c != PICO_ERROR_TIMEOUT) {
			if (c == '\r') {
				// ignore
			} else if (c == '\n') {
				line[pos] = '\0';
				if (pos > 0) {
					// uppercase normalization (simple)
					for (int i = 0; i < pos; ++i) {
						if (line[i] >= 'a' && line[i] <= 'z') line[i] -= 32;
					}

					// handle commands
					if (strcmp(line, "H") == 0 || strcmp(line, "HB") == 0 || strcmp(line, "HEART") == 0) {
						// mark asserted and update last seen
						uint64_t now = to_ms_since_boot(get_absolute_time());
						asserted = true;
						last_seen_ms = now;
						// reset triggered flag if this is a new assertion
						if (!triggered) {
							// no-op; we allow trigger when hold time reached
						}
						// Optional feedback
						printf("SEEN: %s @ %llu\r\n", line, (unsigned long long)now);
					} else if (strcmp(line, "L") == 0 || strcmp(line, "RELEASE") == 0) {
						// explicit release from host
						asserted = false;
						triggered = false;
						printf("RELEASED BY HOST\r\n");
					} else {
						printf("IGNORED CMD: %s\r\n", line);
					}
				}
				pos = 0;
			} else {
				// accumulate character
				if (pos < MAX_LINE - 1) {
					line[pos++] = (char)c;
				} else {
					pos = 0;
					printf("IGNORED: LINE TOO LONG\r\n");
				}
			}
		}

		// periodic state checks: heartbeat expiry -> release
		if (asserted) {
			uint64_t now = to_ms_since_boot(get_absolute_time());
			if (now - last_seen_ms > HEARTBEAT_TIMEOUT_MS) {
				// lost heartbeats -> treat as released
				asserted = false;
				triggered = false;
				printf("RELEASED: heartbeat timeout @ %llu\r\n", (unsigned long long)now);
			} else {
				// still asserted, see if hold duration reached and not yet triggered
				if (!triggered) {
					uint64_t since_assert = now - last_seen_ms + 0; // careful: last_seen_ms is updated on each heartbeat
					// Note: because last_seen_ms is refreshed by heartbeats, we need a slightly different approach:
					// We need the time since the assertion started. To do that we can record assertion_start_ms.
				}
			}
		}

		// To properly measure continuous hold we need assertion_start_ms
		// We'll maintain it below (outside the earlier branch).
		// Slight restructure:

		static uint64_t assertion_start_ms = 0;
		uint64_t now = to_ms_since_boot(get_absolute_time());

		// If we've just become asserted (i.e. asserted==true and assertion_start_ms==0), set start time
		if (asserted && assertion_start_ms == 0) {
			assertion_start_ms = last_seen_ms; // use the timestamp of the first seen message
			// debug:
			printf("ASSERTION START @ %llu\r\n", (unsigned long long)assertion_start_ms);
		}

		// If released, clear the start time and triggered flag
		if (!asserted) {
			if (assertion_start_ms != 0) {
				assertion_start_ms = 0;
			}
			// triggered flag already cleared on release above
		} else {
			// asserted == true
			// check for timeout based release (again)
			if (now - last_seen_ms > HEARTBEAT_TIMEOUT_MS) {
				// timed out
				asserted = false;
				triggered = false;
				assertion_start_ms = 0;
				printf("RELEASED: heartbeat timeout (re-check) @ %llu\r\n", (unsigned long long)now);
			} else if (!triggered && assertion_start_ms != 0) {
				// time since assertion started
				uint64_t held = now - assertion_start_ms;
				if (held >= HOLD_MS) {
					// Trigger next step
					triggered = true;
					printf("HOLD DETECTED: %llu ms -> TRIGGER NEXT STEP\r\n", (unsigned long long)held);
					// --- place your "next step" action here ---
					// Example: flip an internal state, set a flag, or call a function.
					// For demonstration, we'll just print and toggle the on-board LED (if available)
#ifdef PICO_DEFAULT_LED_PIN
					gpio_init(PICO_DEFAULT_LED_PIN);
					gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
					gpio_put(PICO_DEFAULT_LED_PIN, 1);
					sleep_ms(200);
					gpio_put(PICO_DEFAULT_LED_PIN, 0);
#endif
					// ------------------------------------------------
				}
			}
		}

		// small sleep to avoid busy loop
		sleep_ms(5);
	}
	return 0;
}