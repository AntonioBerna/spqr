import matplotlib.pyplot as plt

timeouts = {
    4000: [39653.63, 4727.65, 3500.83, 2714.11, 2206.86, 1694.11, 1304.02, 830.81, 324.29, 75.57, 18.94],
    8000: [32443.26, 3576.49, 2502.25, 1914.10, 1512.28, 1068.69, 926.47, 578.49, 221.31, 50.82, 12.70],
    16000: [28928.45, 2490.50, 1768.88, 1191.13, 892.03, 690.16, 562.95, 355.41, 133.52, 30.73, 7.62],
    32000: [34412.50, 1277.62, 909.38, 655.45, 585.37, 382.73, 309.86, 204.92, 75.07, 17.16, 4.25],
    64000: [42966.73, 835.57, 525.79, 348.69, 267.25, 218.91, 170.61, 108.62, 39.99, 9.09, 2.25],
    80000: [36106.52, 685.13, 425.64, 283.20, 254.48, 164.52, 132.60, 88.31, 32.43, 7.36, 1.35],
}
loss_probability = [0, 5, 10, 15, 20, 25, 30, 40, 60, 80, 90]

plt.figure(figsize=(10, 6))

for timeout, throughput in timeouts.items():
    plt.plot(loss_probability, throughput, marker="o", label=f"Timeout = {timeout} µs")

plt.title("Throughput with Static Timeout and WINDOW_SIZE = 32")
plt.xlabel("Loss Probability (%)")
plt.ylabel("Throughput (KB/s)")
plt.xticks(range(0, 101, 10))
plt.yscale("log")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.legend()

plt.tight_layout()
plt.show()
