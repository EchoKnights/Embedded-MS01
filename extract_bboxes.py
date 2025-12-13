from ultralytics import YOLO
import os
import csv

# -------- CONFIG --------
MODEL_PATH = "D:/AI_Models/NewGen-Recognizor/weights/best.pt"
IMAGE_DIR = "Distance-Regression"
OUTPUT_CSV = "bbox_measurements.csv"
TARGET_CLASS_NAME = "Circle"   # must match your data.yaml name
# ------------------------

model = YOLO(MODEL_PATH)

# Map class index → name
class_names = model.names
target_class_id = None
for k, v in class_names.items():
    if v.lower() == TARGET_CLASS_NAME.lower():
        target_class_id = k
        break

if target_class_id is None:
    raise ValueError(f"Class '{TARGET_CLASS_NAME}' not found in model classes")

rows = []

for img_name in sorted(os.listdir(IMAGE_DIR)):
    if not img_name.lower().endswith((".jpg", ".png", ".jpeg", ".heic")):
        continue

    img_path = os.path.join(IMAGE_DIR, img_name)
    results = model(img_path, verbose=False)

    if len(results) == 0:
        continue

    boxes = results[0].boxes
    if boxes is None:
        continue

    # pick the highest-confidence detection of target class
    best = None
    best_conf = 0.0

    for box in boxes:
        cls = int(box.cls.item())
        conf = float(box.conf.item())

        if cls == target_class_id and conf > best_conf:
            best = box
            best_conf = conf

    if best is None:
        continue

    # YOLO normalized xywh
    x, y, w, h = best.xywhn[0].tolist()
    area = w * h

    rows.append([
        img_name,
        round(w, 6),
        round(h, 6),
        round(area, 6),
        round(best_conf, 4)
    ])

# Write CSV
with open(OUTPUT_CSV, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["image", "bbox_w", "bbox_h", "bbox_area", "confidence"])
    writer.writerows(rows)

print(f"Saved {len(rows)} measurements to {OUTPUT_CSV}")