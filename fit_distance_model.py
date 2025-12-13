import pandas as pd
import numpy as np
from sklearn.linear_model import LinearRegression
from sklearn.metrics import mean_absolute_error, root_mean_squared_error
import joblib
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use("Agg")  # non-GUI backend

# Load data
df = pd.read_csv("bbox_measurements.csv")

# Use bbox height
h = df["bbox_h"].values.reshape(-1, 1)
distance = df["distance_cm"].values

# Inverse feature (physics-inspired)
X = 1.0 / h

# Fit regression
model = LinearRegression()
model.fit(X, distance)

# Predictions
pred = model.predict(X)

# Metrics
mae = mean_absolute_error(distance, pred)
rmse = root_mean_squared_error(distance, pred)

print(f"k (slope): {model.coef_[0]:.3f}")
print(f"b (intercept): {model.intercept_:.3f}")
print(f"MAE:  {mae:.2f} cm")
print(f"RMSE: {rmse:.2f} cm")

plt.scatter(1.0 / h, distance, label="True")
plt.scatter(1.0 / h, pred, label="Predicted")
plt.xlabel("1 / bbox_h")
plt.ylabel("Distance (cm)")
plt.legend()
plt.savefig("distance_fit.png", dpi=150)
plt.close()

# Save model
joblib.dump(model, "distance_regressor.joblib")