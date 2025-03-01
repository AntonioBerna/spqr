import matplotlib.pyplot as plt

data = {
    8: {
        0: (0, 0),
        5: (85, 99),
        10: (127, 155),
        15: (184, 209),
        20: (240, 279),
        25: (331, 348),
        30: (444, 472),
        40: (681, 742),
        60: (1889, 1932),
        80: (7963, 8548),
    },
    16: {
        0: (0, 0),
        5: (56, 67),
        10: (86, 92),
        15: (122, 153),
        20: (169, 189),
        25: (207, 222),
        30: (285, 297),
        40: (453, 477),
        60: (1202, 1230),
        80: (5028, 5507),
    },
    32: {
        0: (0, 0),
        5: (34, 45),
        10: (53, 62),
        15: (73, 88),
        20: (89, 108),
        25: (128, 138),
        30: (164, 181),
        40: (257, 280),
        60: (693, 715),
        80: (2980, 3082),
    },
    64: {
        0: (0, 6),
        5: (24, 30),
        10: (32, 42),
        15: (43, 51),
        20: (62, 69),
        25: (74, 82),
        30: (98, 116),
        40: (151, 163),
        60: (374, 421),
        80: (1682, 1844),
    },
    128: {
        0: (6, 6),
        5: (13, 20),
        10: (23, 25),
        15: (26, 30),
        20: (32, 39),
        25: (42, 48),
        30: (55, 61),
        40: (87, 92),
        60: (140, 218),
        80: (899, 1150),
    }
}

def plot_error_vs_loss_probability():
    plt.figure(figsize=(10, 6))

    colors = ["b", "g", "r", "c", "m"]

    for i, (window_size, error_range) in enumerate(data.items()):
        loss_probabilities = list(error_range.keys())
        min_errors = [error_range[loss][0] for loss in loss_probabilities]
        max_errors = [error_range[loss][1] for loss in loss_probabilities]

        plt.fill_between(loss_probabilities, min_errors, max_errors, color=colors[i], alpha=0.3, label=f"WINDOW_SIZE = {window_size}")

        avg_errors = [(min_e + max_e) / 2 for min_e, max_e in zip(min_errors, max_errors)]
        plt.plot(loss_probabilities, avg_errors, marker="o", color=colors[i], label=f"Avg Error (WINDOW_SIZE = {window_size})")

    plt.title("Cumulative Error (Static Timeout)")
    plt.xlabel("Loss Probability (%)")
    plt.ylabel("Cumulative Error")
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    plot_error_vs_loss_probability()