import matplotlib.pyplot as plt
import numpy as np

loss_probabilities = np.array([0, 5, 10, 15, 20, 25, 30, 40, 60, 80])

cumulative_error_min = np.array([0, 34, 53, 73, 89, 128, 164, 257, 693, 2980])
cumulative_error_max = np.array([0, 45, 62, 88, 108, 138, 181, 280, 715, 3082])

plt.figure(figsize=(10, 6))
plt.plot(loss_probabilities, cumulative_error_min, label="Min Error", color="blue", marker="o")
plt.plot(loss_probabilities, cumulative_error_max, label="Max Error", color="red", marker="o")

plt.fill_between(loss_probabilities, cumulative_error_min, cumulative_error_max, color="gray", alpha=0.3)

plt.title("Cumulative Error (Adaptive Timeout)")
plt.xlabel("Loss Probability (%)")
plt.ylabel("Cumulative Error")

plt.legend()

plt.grid(True)
plt.show()
