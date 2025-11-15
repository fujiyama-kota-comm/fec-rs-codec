import pandas as pd
import matplotlib.pyplot as plt
import os

# =============================================================================
#  Plot BER Curves for NSC Viterbi Decoder
#  ---------------------------------------------------------------------------
#  - Input : results/nsc_ber_data.csv
#      Columns = EbN0_dB, BER_soft, BER_hard, BER_bpsk
#  - Output: images/nsc_ber_graph.png, images/nsc_ber_graph.svg
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
#  Load CSV data
# =============================================================================
df = pd.read_csv("results/nsc_ber_data.csv")

EbN0 = df["EbN0_dB"]
ber_soft = df["BER_soft"]
ber_hard = df["BER_hard"]
ber_bpsk = df["BER_bpsk"]

# =============================================================================
#  Create BER figure
# =============================================================================
plt.figure(figsize=(7.5, 6))

# --- Soft-decision Viterbi --- (緑)
plt.semilogy(
    EbN0,
    ber_soft,
    marker="o",
    markersize=8,
    markerfacecolor="none",
    markeredgewidth=1.8,
    linewidth=2.5,
    color="g",
    label="Soft-decision Viterbi",
)

# --- Hard-decision Viterbi --- (青)
plt.semilogy(
    EbN0,
    ber_hard,
    marker="s",
    markersize=8,
    markerfacecolor="none",
    markeredgewidth=1.8,
    linewidth=2.5,
    color="b",
    label="Hard-decision Viterbi",
)

# --- Uncoded BPSK (赤)
plt.semilogy(
    EbN0,
    ber_bpsk,
    linewidth=2.5,
    color="r",
    label="Uncoded BPSK (theory)",
)

# Y 軸スケール
plt.ylim(1e-5, 1)

# 軸ラベル
plt.xlabel("Eb/N0 [dB]", fontsize=18)
plt.ylabel("Bit Error Rate (BER)", fontsize=18)

# グリッド（対数軸向け）
plt.grid(True, which="both", linestyle="--", linewidth=0.6, alpha=0.6)

# 凡例
plt.legend(fontsize=14, loc="upper right", frameon=True, edgecolor="black")

plt.tight_layout()

# =============================================================================
#  Save figure
# =============================================================================
plt.savefig("images/nsc_ber_graph.png", dpi=300, bbox_inches="tight")  # PNG
plt.savefig("images/nsc_ber_graph.svg", dpi=300, bbox_inches="tight")  # SVG (vector)

plt.show()
