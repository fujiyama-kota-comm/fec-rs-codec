import pandas as pd
import matplotlib.pyplot as plt
import os
import re

# =============================================================================
#  Plot BER/BLER Curves for Reed–Solomon over AWGN (BPSK)
# =============================================================================

os.makedirs("images", exist_ok=True)

# =============================================================================
#  Matplotlib font settings
# =============================================================================
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams["mathtext.fontset"] = "stix"
plt.rcParams["font.size"] = 14

# =============================================================================
#  Extract m, N, K from file name
# =============================================================================


def extract_params(filename):
    """
    Filename example:
        rs_ber_m8_N255_K223.csv
    """
    pattern = r"m(\d+)_N(\d+)_K(\d+)"
    match = re.search(pattern, filename)
    if match:
        m, N, K = match.groups()
        return int(m), int(N), int(K)
    return None, None, None


# =============================================================================
#  Load BER CSV
# =============================================================================

ber_files = [f for f in os.listdir("results") if f.startswith("rs_ber")]
if len(ber_files) == 0:
    raise FileNotFoundError("No rs_ber_m*_N*_K*.csv found in results/")

ber_file = os.path.join("results", ber_files[0])
m, N, K = extract_params(ber_file)

df_ber = pd.read_csv(ber_file)
EbN0 = df_ber["EbN0_dB"]
ber_rs = df_ber["BER_RS"]
ber_bpsk = df_ber["BER_bpsk"]

# =============================================================================
#  BER Plot
# =============================================================================

plt.figure(figsize=(7.5, 6))

plt.semilogy(
    EbN0,
    ber_rs,
    marker="o",
    markersize=8,
    markerfacecolor="none",
    markeredgewidth=1.8,
    linewidth=2.5,
    color="g",
    label=f"RS({N},{K}) coded BPSK",
)

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

# ---- Annotation: only m ----
plt.annotate(
    f"m={m}",
    xy=(0.97, 0.03),
    xycoords="axes fraction",
    ha="right",
    va="bottom",
    fontsize=12,
    bbox=dict(
        boxstyle="round,pad=0.35",
        edgecolor="black",
        facecolor="white",
        alpha=0.85,
    ),
)

plt.tight_layout()
plt.savefig("images/rs_ber_graph.png", dpi=300, bbox_inches="tight")
plt.savefig("images/rs_ber_graph.svg", dpi=300, bbox_inches="tight")
plt.show()


# =============================================================================
#  Load BLER CSV
# =============================================================================

bler_files = [f for f in os.listdir("results") if f.startswith("rs_bler")]
if len(bler_files) == 0:
    raise FileNotFoundError("No rs_bler_m*_N*_K*.csv found in results/")

bler_file = os.path.join("results", bler_files[0])
df_bler = pd.read_csv(bler_file)

EbN0_b = df_bler["EbN0_dB"]
bler_rs = df_bler["BLER_RS"]
bler_bpsk = df_bler["BLER_bpsk"]

# =============================================================================
#  BLER Plot
# =============================================================================

plt.figure(figsize=(7.5, 6))

plt.semilogy(
    EbN0_b,
    bler_rs,
    marker="s",
    markersize=8,
    markerfacecolor="none",
    markeredgewidth=1.8,
    linewidth=2.5,
    color="g",
    label=f"RS({N},{K}) coded BPSK",
)

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

# ---- Annotation: only m ----
plt.annotate(
    f"m={m}",
    xy=(0.97, 0.03),
    xycoords="axes fraction",
    ha="right",
    va="bottom",
    fontsize=12,
    bbox=dict(
        boxstyle="round,pad=0.35",
        edgecolor="black",
        facecolor="white",
        alpha=0.85,
    ),
)

plt.tight_layout()
plt.savefig("images/rs_bler_graph.png", dpi=300, bbox_inches="tight")
plt.savefig("images/rs_bler_graph.svg", dpi=300, bbox_inches="tight")
plt.show()
