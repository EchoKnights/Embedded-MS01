# Import required libraries
from ultralytics import YOLO
import cv2
import sys, time
import serial
import serial.tools.list_ports

# ---------------- CONFIG ----------------
MODEL_PATH = "./best.pt"
K_VALUE = 12.263 *2          # your fitted constant
CONF_THRESHOLD = 0.75
# ----------------------------------------

# Load model
model = YOLO(MODEL_PATH)

# Create window
cv2.namedWindow("YOLOv11-S Live", cv2.WINDOW_NORMAL)

# Open webcam
cap = cv2.VideoCapture(0)

HEARTBEAT_MS = 30

DEFAULT_CANDIDATES = ["ttyACM", "ttyUSB", "usbmodem", "COM"]

def find_port():
	if len(sys.argv) > 1:
		return sys.argv[1]
	ports = list(serial.tools.list_ports.comports())
	for p in ports:
		n = (p.device + " " + (p.description or "")).lower()
		for cand in DEFAULT_CANDIDATES:
			if cand.lower() in n:
				return p.device
	if ports:
		return ports[0].device
	return None

def assert_hold(ser, hold_seconds):
	# send initial assert
	ser.write(b"H\n")
	start = time.time()
	next_hb = start + (HEARTBEAT_MS/1000.0)
	while time.time() - start < hold_seconds:
		now = time.time()
		if now >= next_hb:
			ser.write(b"HB\n")
			next_hb += (HEARTBEAT_MS/1000.0)
		# optional: read replies, print them
		try:
			while ser.in_waiting:
				line = ser.readline().decode('utf-8', errors='ignore').strip()
				if line:
					print("PICO:", line)
		except Exception:
			pass
		time.sleep(0.01)
	# send explicit release
	ser.write(b"L\n'")

def yolo_read():
	with serial.Serial("COM8", 115200, timeout=0.1) as ser:
		while True:
			ret, frame = cap.read()
			if not ret:
				break
			
			# Run detection
			results = model(frame, conf=CONF_THRESHOLD)

			annotated = frame.copy()

			for box in results[0].boxes:
				# Bounding box in pixels
				x1, y1, x2, y2 = map(int, box.xyxy[0])

				# Normalized box height (0..1)
				box_height = float(box.xywhn[0][3])

				# Avoid division by zero
				if box_height <= 0:
					continue

				# Distance estimation
				distance = K_VALUE / box_height

				# Draw bounding box
				cv2.rectangle(annotated, (x1, y1), (x2, y2), (0, 255, 0), 2)

				# Label text
				label = f"{distance:.1f}"

				# Draw label background
				(tw, th), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.6, 2)
				cv2.rectangle(annotated, (x1, y1 - th - 6), (x1 + tw + 6, y1), (0, 255, 0), -1)

				# Draw label text
				cv2.putText(
					annotated,
					label,
					(x1 + 3, y1 - 4),
					cv2.FONT_HERSHEY_SIMPLEX,
					0.6,
					(0, 0, 0),
					2
				)

				# ser.write(f"{distance:.1f}\n")
				# line = ser.readline().decode(errors="ignore").strip()
				# print("PICO:", line)
			
			# if(len(results[0].boxes) == 0):
			# 	ser.write(b"94.2\n")
			# 	line = ser.readline().decode(errors="ignore").strip()
			# 	print("PICO:", line)

			# Show result
			cv2.imshow("YOLOv11-S Live", annotated)

			if cv2.waitKey(1) & 0xFF == ord('q'):
				break

			time.sleep(0.05)  # 20 Hz

if __name__ == "__main__":
	# port = find_port()
	# if not port:
	# 	print("No serial port found. Plug in Pico or pass port as arg.")
	# 	sys.exit(1)
	# print("Using port", port)
	yolo_read()
	cap.release()
	cv2.destroyAllWindows()

