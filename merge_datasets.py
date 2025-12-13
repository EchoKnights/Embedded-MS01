import os
import shutil

# --------------------------
# CONFIGURATION
# --------------------------

# Paths to original datasets
SYNTHETIC_DIR = "Training-Synthetic"
REALWORLD_DIR = "Training-Realworld"

# Path to new merged dataset
MERGED_DIR = "Training-Real-Synthetic"

# Final class order (indices = 0..5)
FINAL_CLASSES = ['Circle', 'Hexagon', 'Rhombus', 'Square', 'Star', 'Triangle']

# Old classes from synthetic dataset
OLD_CLASSES = ['Circle', 'Ellipse', 'Hexagon', 'Pentagon', 'Quatrefoil',
               'Rectangle', 'Rhombus', 'Square', 'Star', 'Triangle']

# Map old class index -> new class index
CLASS_MAP = {}
for i, cls in enumerate(OLD_CLASSES):
    if cls in FINAL_CLASSES:
        CLASS_MAP[i] = FINAL_CLASSES.index(cls)
# Old classes not in FINAL_CLASSES will be skipped automatically

# Subfolders to process
SPLITS = ['train', 'valid', 'test']

# --------------------------
# CREATE MERGED DIRECTORY
# --------------------------

for split in SPLITS:
    os.makedirs(os.path.join(MERGED_DIR, split, "images"), exist_ok=True)
    os.makedirs(os.path.join(MERGED_DIR, split, "labels"), exist_ok=True)

# --------------------------
# FUNCTION TO MERGE DATASET
# --------------------------

def merge_dataset(src_dir, dataset_name):
    for split in SPLITS:
        src_img_dir = os.path.join(src_dir, split, "images")
        src_lbl_dir = os.path.join(src_dir, split, "labels")
        dst_img_dir = os.path.join(MERGED_DIR, split, "images")
        dst_lbl_dir = os.path.join(MERGED_DIR, split, "labels")

        lbl_files = [f for f in os.listdir(src_lbl_dir) if f.endswith(".txt")]
        for lbl_file in lbl_files:
            src_lbl_path = os.path.join(src_lbl_dir, lbl_file)
            dst_lbl_path = os.path.join(dst_lbl_dir, lbl_file)

            # Process label lines
            new_lines = []
            with open(src_lbl_path, "r") as f:
                for line in f:
                    tokens = line.strip().split()
                    if not tokens:
                        continue
                    old_idx = int(tokens[0])
                    if old_idx in CLASS_MAP:
                        new_idx = CLASS_MAP[old_idx]
                        new_lines.append(f"{new_idx} " + " ".join(tokens[1:]))

            # Write remapped labels
            with open(dst_lbl_path, "w") as f:
                for l in new_lines:
                    f.write(l + "\n")

            # Copy corresponding image
            src_img_path = os.path.join(src_img_dir, lbl_file.replace(".txt", ".jpg"))
            if not os.path.exists(src_img_path):
                src_img_path = os.path.join(src_img_dir, lbl_file.replace(".txt", ".png"))
            if os.path.exists(src_img_path):
                shutil.copy(src_img_path, dst_img_dir)

        print(f"Finished refactoring of {dataset_name} {split.capitalize()} Directory Labels")

# --------------------------
# MERGE SYNTHETIC
# --------------------------
merge_dataset(SYNTHETIC_DIR, "Synthetic")

# --------------------------
# MERGE REALWORLD
# --------------------------
merge_dataset(REALWORLD_DIR, "Realworld")

# --------------------------
# CREATE NEW DATA.YAML
# --------------------------
data_yaml_path = os.path.join(MERGED_DIR, "data.yaml")
with open(data_yaml_path, "w") as f:
    f.write(f"train: {os.path.join(MERGED_DIR, 'train', 'images')}\n")
    f.write(f"val:   {os.path.join(MERGED_DIR, 'valid', 'images')}\n")
    f.write(f"test:  {os.path.join(MERGED_DIR, 'test', 'images')}\n\n")
    f.write(f"nc: {len(FINAL_CLASSES)}\n")
    f.write(f"names: {FINAL_CLASSES}\n")

print(f"Merged dataset created at '{MERGED_DIR}' with new data.yaml")
