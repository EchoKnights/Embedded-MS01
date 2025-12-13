# Import required libraries
from ultralytics import YOLO
import cv2

# ---------------- CONFIG ----------------
MODEL_PATH = "D:/AI_Models/NewGen-Recognizor-O2/weights/best.pt"
K_VALUE = 12.263 *2          # your fitted constant
CONF_THRESHOLD = 0.75
# ----------------------------------------

# Load model
model = YOLO(MODEL_PATH)

# Create window
cv2.namedWindow("YOLOv11-S Live", cv2.WINDOW_NORMAL)

# Open webcam
cap = cv2.VideoCapture(0)

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
		label = f"{distance:.2f}"

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

	# Show result
	cv2.imshow("YOLOv11-S Live", annotated)

	if cv2.waitKey(1) & 0xFF == ord('q'):
		break

cap.release()
cv2.destroyAllWindows()