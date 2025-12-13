import cv2
import numpy as np
from ultralytics import YOLO
import sys, time
import serial
import serial.tools.list_ports

# =========================
# CONFIG (edit these)
# =========================
MODEL_PATH = r"best.pt"
WEBCAM_INDEX = 0

IMGSZ = 640
CONF = 0.35
MAX_DET = 3
DEVICE = None  # None=auto, or "cpu", or "0" for GPU 0

WINDOW_NAME = "YOLO Circle Distance (ESC quit)"

# --- Your circle: radius is always 9cm ---
RADIUS_CM = 9.0
DIAMETER_CM = RADIUS_CM * 2.0  # = 18cm

# --- One-time calibration ---
# Put the circle at this distance and press SPACE once.
CALIB_DIST_CM = 50.0

# After you calibrate once, paste the printed value here:
FOCAL_PX = None  # e.g. 812.43

# --- Puzzle range ---
TARGET_MIN_CM = 90.0
TARGET_MAX_CM = 95.0

# --- Smoothing ---
SMOOTH_N = 8
# =========================


def bbox_pixel_diameter(x1, y1, x2, y2):
    """Approx circle pixel diameter from bbox size."""
    w = max(1, x2 - x1)
    h = max(1, y2 - y1)
    return (w + h) / 2.0


def main():
    with serial.Serial("COM8", 115200, timeout=0.1) as ser:
        global FOCAL_PX

        model = YOLO(MODEL_PATH)

        cap = cv2.VideoCapture(WEBCAM_INDEX)
        if not cap.isOpened():
            raise RuntimeError(f"Could not open webcam index {WEBCAM_INDEX}. Try WEBCAM_INDEX=1")

        dist_hist = []

        print("Controls: ESC quit | SPACE calibrate (only if FOCAL_PX is None)")

        if FOCAL_PX is None:
            print(f"Calibration mode: Hold circle at {CALIB_DIST_CM:.1f} cm then press SPACE.")
            print(f"Circle diameter assumed: {DIAMETER_CM:.1f} cm")

            while True:
                ok, frame = cap.read()
                if not ok:
                    print("Failed to read from webcam.")
                    break

                results = model.predict(
                    source=frame,
                    imgsz=IMGSZ,
                    conf=CONF,
                    max_det=MAX_DET,
                    device=DEVICE,
                    verbose=False
                )[0]

                annotated = frame.copy()

                # pick best detection (highest confidence)
                best = None
                best_conf = -1.0
                if results.boxes is not None and len(results.boxes) > 0:
                    for b in results.boxes:
                        conf = float(b.conf[0].cpu().numpy())
                        if conf > best_conf:
                            best_conf = conf
                            best = b

                if best is not None:
                    x1, y1, x2, y2 = best.xyxy[0].cpu().numpy().astype(int)
                    d_px = bbox_pixel_diameter(x1, y1, x2, y2)

                    # draw bbox
                    cv2.rectangle(annotated, (x1, y1), (x2, y2), (0, 255, 0), 2)
                    cv2.putText(
                        annotated,
                        f"conf={best_conf:.2f} d={d_px:.1f}px",
                        (x1, max(0, y1 - 10)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2
                    )

                    if FOCAL_PX is None:
                        cv2.putText(
                            annotated,
                            f"CALIBRATE: hold at {CALIB_DIST_CM:.0f}cm, press SPACE",
                            (20, 40),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 255), 2
                        )
                    else:
                        # distance estimate
                        Z_cm = (FOCAL_PX * DIAMETER_CM) / max(d_px, 1e-6)

                        # smooth
                        dist_hist.append(Z_cm)
                        if len(dist_hist) > SMOOTH_N:
                            dist_hist.pop(0)
                        Z_smooth = float(np.mean(dist_hist))

                        ser.write(f"{Z_smooth:.1f}\n")
                        line = ser.readline().decode(errors="ignore").strip()
                        print("PICO:", line)

                        in_range = (TARGET_MIN_CM <= Z_smooth <= TARGET_MAX_CM)

                        cv2.putText(
                            annotated,
                            f"Distance: {Z_smooth:.1f} cm",
                            (20, 40),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.9,
                            (0, 255, 0), 2
                        )
                        cv2.putText(
                            annotated,
                            f"IN RANGE: {'YES' if in_range else 'NO'}  [{TARGET_MIN_CM:.0f}-{TARGET_MAX_CM:.0f} cm]",
                            (20, 80),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8,
                            (0, 255, 0) if in_range else (0, 0, 255), 2
                        )

                else:
                    dist_hist.clear()
                    cv2.putText(
                        annotated,
                        "Circle: LOST",
                        (20, 40),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.9,
                        (0, 0, 255), 2
                    )

                cv2.imshow(WINDOW_NAME, annotated)

                key = cv2.waitKey(1) & 0xFF
                if key == 27:  # ESC
                    break

                # SPACE to calibrate (only if we are not calibrated yet and a circle exists)
                if key == 32 and (FOCAL_PX is None) and (best is not None):
                    x1, y1, x2, y2 = best.xyxy[0].cpu().numpy().astype(int)
                    d0_px = bbox_pixel_diameter(x1, y1, x2, y2)

                    FOCAL_PX = (d0_px * CALIB_DIST_CM) / DIAMETER_CM

                    print("\n=== CALIBRATION DONE ===")
                    print(f"d0_px (measured) = {d0_px:.2f} px")
                    print(f"FOCAL_PX = {FOCAL_PX:.2f}")
                    print("Paste this into the script (FOCAL_PX = ...) so next runs are automatic.")
                    print("========================\n")

                time.sleep(0.25)

        cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
