import pandas as pd
import matplotlib.pyplot as plt
import os

# =============================================================================
#  Plot BER/BLER Curves for Reed–Solomon over AWGN (BPSK)
# =============================================================================

# ▼ 出力フォルダ（images/）が無い場合は自動作成
os.makedirs("images", exist_ok=True)

# =============================================================================
#  Matplotlib フォント設定（論文向け）
# =============================================================================
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams["mathtext.fontset"] = "stix"
plt.rcParams["font.size"] = 14

# =============================================================================
#  1) Load BER CSV data
# =============================================================================
df_ber = pd.read_csv("results/rs_ber_data.csv")
EbN0 = df_ber["EbN0_dB"]
ber_rs = df_ber["BER_RS"]
ber_bpsk = df_ber["BER_bpsk"]

# =============================================================================
#  2) Plot BER graph
# =============================================================================
plt.figure(figsize=(7.5, 6))

# --- RS-coded BPSK
plt.semilogy(
    EbN0,
    ber_rs,
    marker="o",
    markersize=8,
    markerfacecolor="none",
    markeredgewidth=1.8,
    linewidth=2.5,
    color="g",
    label="Reed-Solomon BPSK",
)

# --- Uncoded BPSK
plt.semilogy(
    EbN0,
    ber_bpsk,
    linewidth=2.5,
    color="r",
    label="Uncoded BPSK (theory)",
)

plt.ylim(1e-5, 1)
plt.xlabel("Eb/N0 [dB]", fontsize=18)
plt.ylabel("Bit Error Rate (BER)", fontsize=18)
plt.grid(True, which="both", linestyle="--", linewidth=0.6, alpha=0.6)
plt.legend(fontsize=14, loc="upper right", frameon=True, edgecolor="black")
plt.tight_layout()

# --- Save BER figure
plt.savefig("images/rs_ber_graph.png", dpi=300, bbox_inches="tight")
plt.savefig("images/rs_ber_graph.svg", dpi=300, bbox_inches="tight")

plt.show()


# =============================================================================
#  3) Load BLER CSV data
# =============================================================================
df_bler = pd.read_csv("results/rs_bler_data.csv")
EbN0_b = df_bler["EbN0_dB"]
bler_rs = df_bler["BLER_RS"]
bler_bpsk = df_bler["BLER_bpsk"]  # 必要なら BPSK の理論BLER列を作っても良い

# =============================================================================
#  4) Plot BLER graph
# =============================================================================
plt.figure(figsize=(7.5, 6))

# --- RS-coded BPSK
plt.semilogy(
    EbN0_b,
    bler_rs,
    marker="s",
    markersize=8,
    markerfacecolor="none",
    markeredgewidth=1.8,
    linewidth=2.5,
    color="g",
    label="Reed-Solomon BPSK",
)

# --- Uncoded BPSK
plt.semilogy(
    EbN0_b,
    bler_bpsk,
    linewidth=2.5,
    color="r",
    label="Uncoded BPSK (theory)",
)

plt.ylim(1e-5, 1)
plt.xlabel("Eb/N0 [dB]", fontsize=18)
plt.ylabel("Block Error Rate (BLER)", fontsize=18)
plt.grid(True, which="both", linestyle="--", linewidth=0.6, alpha=0.6)
plt.legend(fontsize=14, loc="upper right", frameon=True, edgecolor="black")
plt.tight_layout()

# --- Save BLER figure
plt.savefig("images/rs_bler_graph.png", dpi=300, bbox_inches="tight")
plt.savefig("images/rs_bler_graph.svg", dpi=300, bbox_inches="tight")

plt.show()
