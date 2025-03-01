import matplotlib.pyplot as plt

loss_probability = [0, 5, 10, 15, 20, 25, 30, 40, 60, 80]

timeout_adaptive = [34971.78, 3653.44, 2318.92, 1831.08, 1404.64, 993.31, 842.40, 518.07, 135.48, 7.49]
static_timeout_8000 = [32443.26, 3576.49, 2502.25, 1914.10, 1512.28, 1068.69, 926.47, 578.49, 221.31, 50.82]
static_timeout_32000 = [34412.50, 1277.62, 909.38, 655.45, 585.37, 382.73, 309.86, 204.92, 75.07, 17.16]
static_timeout_80000 = [36106.52, 685.13, 425.64, 283.20, 254.48, 164.52, 132.60, 88.31, 32.43, 7.36]

plt.figure(figsize=(10, 6))

plt.plot(loss_probability, timeout_adaptive, label="Adaptive Timeout", marker="o")
plt.plot(loss_probability, static_timeout_8000, label="Static Timeout 8000 µs", marker="s")
plt.plot(loss_probability, static_timeout_32000, label="Static Timeout 32000 µs", marker="^")
plt.plot(loss_probability, static_timeout_80000, label="Static Timeout 80000 µs", marker="d")

plt.title("Adaptive Timeout vs Static Timeout")
plt.xlabel("Loss Probability (%)")
plt.ylabel("Throughput (KB/s)")
plt.yscale("log")
plt.grid(which="both", linestyle="--", linewidth=0.5)
plt.legend()

plt.tight_layout()
plt.show()
