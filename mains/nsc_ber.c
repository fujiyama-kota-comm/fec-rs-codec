#include "version.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h> /* Windows の _mkdir */
#else
#include <sys/stat.h> /* POSIX mkdir */
#include <sys/types.h>
#endif

#include "nsc_decoder.h"
#include "nsc_encoder.h"

#define PI 3.14159265358979323846

int trials = 100000; /* Monte Carlo 試行回数 */
double EbN0_min = 0.0;
double EbN0_max = 10.0;
double EbN0_step = 1.0;

/* ============================================================================
 *  Gaussian noise generator (Box–Muller transform)
 *  --------------------------------------------------------------------------
 *  randn() は正規分布 N(0,1) に従う乱数を生成する。
 *  AWGN チャネル実装で使用。
 * ========================================================================== */
double randn() {
  double u1 = (rand() + 1.0) / (RAND_MAX + 2.0);
  double u2 = (rand() + 1.0) / (RAND_MAX + 2.0);
  return sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
}

/* ============================================================================
 *  Theoretical BER of BPSK over AWGN
 *  --------------------------------------------------------------------------
 *     BER = Q( sqrt(2 * Eb/N0) )
 *          = 0.5 * erfc( sqrt(Eb/N0) )
 *
 *  コーディング無しの BPSK 性能との比較のために使用。
 * ========================================================================== */
double bpsk_ber(double EbN0_linear) {
  return 0.5 * erfc(sqrt(2.0 * EbN0_linear) / sqrt(2.0));
}

/* ============================================================================
 *  MAIN: BER simulation for NSC Viterbi Decoder
 *  --------------------------------------------------------------------------
 *  - Rate=1/2 NSC encoder/decoder に対する BER 実験
 *  - チャネル：AWGN (BPSK 変調)
 *  - 評価：Soft-decision / Hard-decision Viterbi の BER
 *  - 出力：results/nsc_ber_data.csv に保存
 *
 *  CSV フォーマット：
 *      EbN0_dB, BER_soft, BER_hard, BER_bpsk
 * ========================================================================== */
int main() {
  printf("fec-nsc-codec version %s\n", NSC_VERSION);

  /* ----------------------------------------------------------------------
   * Create results/ directory
   * ------------------------------------------------------------------ */
#ifdef _WIN32
  _mkdir("results");
#else
  mkdir("results", 0777);
#endif

  /* ----------------------------------------------------------------------
   * Simulation parameters
   * ------------------------------------------------------------------ */
  nsc_info_len = 100;           /* 情報ビット長 K              */
  nsc_code_len = 2 * (100 + 2); /* 符号長 N = 2*(K + tail_len) */

  int K = nsc_info_len;
  int N = nsc_code_len;

  /* ----------------------------------------------------------------------
   * Open CSV file
   * ------------------------------------------------------------------ */
  FILE *fp = fopen("results/nsc_ber_data.csv", "w");
  if (!fp) {
    fprintf(stderr, "Cannot open results/nsc_ber_data.csv\n");
    return -1;
  }
  fprintf(fp, "EbN0_dB,BER_soft,BER_hard,BER_bpsk\n");

  /* ----------------------------------------------------------------------
   * Memory allocation
   * ------------------------------------------------------------------ */
  int *data = malloc(sizeof(int) * K);
  int *code = malloc(sizeof(int) * N);
  double *LLR = malloc(sizeof(double) * N);
  int *rx_bits = malloc(sizeof(int) * N);
  int *info_soft = malloc(sizeof(int) * K);
  int *info_hard = malloc(sizeof(int) * K);
  int *code_hat = malloc(sizeof(int) * N); /* optional: for consistency check */

  if (!data || !code || !LLR || !rx_bits || !info_soft || !info_hard ||
      !code_hat) {
    fprintf(stderr, "Memory allocation failed\n");
    return -1;
  }

  /* ----------------------------------------------------------------------
   * RNG initialize
   * ------------------------------------------------------------------ */
  srand((unsigned)time(NULL));

  printf("EbN0_dB, BER_soft, BER_hard, BER_bpsk\n");

  /* ==========================================================================
   *  Main Eb/N0 sweep
   * ======================================================================== */
  for (double EbN0_dB = EbN0_min; EbN0_dB <= EbN0_max; EbN0_dB += EbN0_step) {

    double EbN0 = pow(10.0, EbN0_dB / 10.0);
    double R = 0.5;                         /* code rate */
    double sigma2 = 1.0 / (2.0 * R * EbN0); /* AWGN noise variance */
    double sigma = sqrt(sigma2);

    long total_bits = 0;
    long error_soft = 0;
    long error_hard = 0;

    /* ------------------------------------------------------------------
     *  Monte Carlo simulation loop
     * ---------------------------------------------------------------- */
    for (int t = 0; t < trials; t++) {

      /* ---------------- Random information bits ------------------- */
      for (int i = 0; i < K; i++)
        data[i] = rand() & 1;

      /* ---------------------- Encode ------------------------------- */
      nsc_encode_r05(data, code);

      /* -------------------- AWGN channel --------------------------- */
      for (int i = 0; i < N; i++) {

        /* BPSK mapping */
        double s = (code[i] == 0) ? +1.0 : -1.0;

        /* Add noise */
        double y = s + sigma * randn();

        /* Soft LLR   : LLR = 2*y / σ^2  (BPSK AWGN の対数尤度比) */
        LLR[i] = (2.0 * y) / sigma2;

        /* Hard判定   : y>=0 → 0, y<0 → 1 */
        rx_bits[i] = (y >= 0.0) ? 0 : 1;
      }

      /* ---------------- Soft/Hard Viterbi -------------------------- */
      nsc_decode_r05_soft(LLR, info_soft, code_hat);
      nsc_decode_r05_hard(rx_bits, info_hard, code_hat);

      /* ---------------- Count bit errors --------------------------- */
      for (int i = 0; i < K; i++) {
        if (info_soft[i] != data[i])
          error_soft++;
        if (info_hard[i] != data[i])
          error_hard++;
      }

      total_bits += K;
    }

    /* ------------------------------------------------------------------
     *  BER evaluation
     * ---------------------------------------------------------------- */
    double ber_soft = (double)error_soft / total_bits;
    double ber_hard = (double)error_hard / total_bits;
    double ber_bpsk = bpsk_ber(EbN0);

    printf("%.1f, %.10f, %.10f, %.10f\n", EbN0_dB, ber_soft, ber_hard,
           ber_bpsk);
    fprintf(fp, "%.1f,%.10f,%.10f,%.10f\n", EbN0_dB, ber_soft, ber_hard,
            ber_bpsk);
  }

  /* ----------------------------------------------------------------------
   * Cleanup
   * ------------------------------------------------------------------ */
  fclose(fp);

  free(data);
  free(code);
  free(LLR);
  free(rx_bits);
  free(info_soft);
  free(info_hard);
  free(code_hat);

  return 0;
}
