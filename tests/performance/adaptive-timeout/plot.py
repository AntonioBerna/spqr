import matplotlib.pyplot as plt

data = {
    8: [34625.34, 1670.61, 965.88, 695.77, 521.22, 373.66, 312.59, 187.29, 12.99, 2.88],
    16: [35602.67, 2394.67, 1546.64, 1078.24, 827.01, 622.92, 500.43, 339.71, 41.44, 4.59],
    32: [34971.78, 3653.44, 2318.92, 1831.08, 1404.64, 993.31, 842.40, 518.07, 135.48, 7.49],
    64: [32822.13, 6061.48, 3808.67, 2836.82, 2348.13, 1827.67, 1495.68, 1037.10, 312.31, 17.72],
    128: [16168.72, 7704.30, 5816.49, 4606.03, 3766.66, 2970.95, 2421.00, 1778.96, 589.83, 54.92]
}

loss_probabilities = [0, 5, 10, 15, 20, 25, 30, 40, 60, 80]

plt.figure(figsize=(10, 6))

for window_size, throughput_values in data.items():
    plt.plot(loss_probabilities, throughput_values, marker="o", label=f"WINDOW_SIZE = {window_size}")

plt.title("Throughput with Adaptive Timeout")
plt.xlabel("Loss Probability (%)")
plt.ylabel("Throughput (KB/s)")
plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()

plt.tight_layout()
plt.show()
