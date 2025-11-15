/**
 * @file rs_ber_bler.c
 * @brief Reed–Solomon BER/BLER simulation over AWGN (BPSK, hard decision).
 *
 * Output:
 *   results/rs_ber_data.csv   (EbN0_dB, BER_RS, BER_bpsk)
 *   results/rs_bler_data.csv  (EbN0_dB, BLER_RS, BLER_bpsk)
 *
 * Assumptions:
 *   - RS code over GF(2^m)
 *   - Bit-interleaved BPSK over AWGN
 *   - Hard-decision demodulation → RS hard-decoder
 *
 * Required API:
 *   void rs_gf_init(int m, int N, int K, int T);
 *   void rs_encode(const int *u_bits, int *c_bits);
 *   void rs_decode(const int *r_bits, int *c_hat_bits, int *u_hat_bits);
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "rs_decoder.h"
#include "rs_encoder.h"
#include "rs_gf.h"

#define PI 3.141592653589793

/* ------------------------------------------------------------------------- */
/* Simulation parameters                                                     */
/* ------------------------------------------------------------------------- */
static const int RS_M = 8;   /* GF(2^m) */
static const int RS_N = 255; /* codeword length (symbols)   */
static const int RS_K = 223; /* information length (symbols)*/

static const int N_TRIALS = 100000; /* frames per SNR              */
static const double EbN0_MIN_dB = 0.0;
static const double EbN0_MAX_dB = 14.0;
static const double EbN0_STEP_dB = 0.5;

/* ------------------------------------------------------------------------- */
/* Utility: Gaussian random (Box–Muller)                                     */
/* ------------------------------------------------------------------------- */
static double rand_uniform(void) { return (rand() + 1.0) / (RAND_MAX + 2.0); }

static double randn(void) {
  double u1 = rand_uniform();
  double u2 = rand_uniform();
  return sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
}

/* ------------------------------------------------------------------------- */
/* BPSK theoretical BER: BER = 0.5 * erfc( sqrt(Eb/N0) )                     */
/* ------------------------------------------------------------------------- */
static double bpsk_ber(double EbN0_linear) {
  return 0.5 * erfc(sqrt(EbN0_linear));
}

/* ======================================================================== */
/* MAIN                                                                     */
/* ======================================================================== */
int main(void) {

  printf("=====================================================\n");
  printf("  Reed–Solomon BER/BLER Simulation over AWGN (BPSK)  \n");
  printf("=====================================================\n\n");

  int m = RS_M;
  int N = RS_N;
  int K = RS_K;
  int T = N - K; /* parity symbols */
  int code_bits_len = N * m;
  int info_bits_len = K * m;

  printf("RS parameters:\n");
  printf("  GF(2^m) : m = %d\n", m);
  printf("  Code    : RS(%d, %d), parity symbols T = %d\n", N, K, T);
  printf("  Trials  : %d frames per SNR point\n\n", N_TRIALS);

  if (rs_gf_init(m, N, K, T) != 0) {
    fprintf(stderr, "rs_gf_init failed.\n");
    return 1;
  }

#ifdef _WIN32
  _mkdir("results");
#else
  mkdir("results", 0777);
#endif

  FILE *fp = fopen("results/rs_ber_data.csv", "w");
  FILE *fp_bler = fopen("results/rs_bler_data.csv", "w");
  if (!fp || !fp_bler) {
    fprintf(stderr, "Cannot open results CSV files\n");
    return 1;
  }
  fprintf(fp, "EbN0_dB,BER_RS,BER_bpsk\n");
  fprintf(fp_bler, "EbN0_dB,BLER_RS,BLER_bpsk\n");

  int *u_bits = (int *)malloc(info_bits_len * sizeof(int));
  int *c_bits = (int *)malloc(code_bits_len * sizeof(int));
  int *r_bits = (int *)malloc(code_bits_len * sizeof(int));
  int *c_hat = (int *)malloc(code_bits_len * sizeof(int));
  int *u_hat = (int *)malloc(info_bits_len * sizeof(int));
  double *tx = (double *)malloc(code_bits_len * sizeof(double));
  double *rx = (double *)malloc(code_bits_len * sizeof(double));

  if (!u_bits || !c_bits || !r_bits || !c_hat || !u_hat || !tx || !rx) {
    fprintf(stderr, "Memory allocation failed.\n");
    return 1;
  }

  srand((unsigned int)time(NULL));

  printf("EbN0_dB, BER_RS, BER_bpsk, BLER_RS, BLER_bpsk\n");

  for (double EbN0_dB = EbN0_MIN_dB; EbN0_dB <= EbN0_MAX_dB + 1e-9;
       EbN0_dB += EbN0_STEP_dB) {

    double EbN0 = pow(10.0, EbN0_dB / 10.0);
    double R = (double)K / (double)N;
    double sigma2 = 1.0 / (2.0 * R * EbN0);
    double sigma = sqrt(sigma2);

    long long total_info_bits = (long long)N_TRIALS * info_bits_len;
    long long err_info = 0;
    long long sum_frame_errors = 0;

    for (int t = 0; t < N_TRIALS; t++) {
      for (int i = 0; i < info_bits_len; i++) {
        u_bits[i] = rand() & 1;
      }

      rs_encode(u_bits, c_bits);

      for (int i = 0; i < code_bits_len; i++) {
        tx[i] = (c_bits[i] == 1) ? +1.0 : -1.0;
      }

      for (int i = 0; i < code_bits_len; i++) {
        double n = sigma * randn();
        rx[i] = tx[i] + n;
      }

      for (int i = 0; i < code_bits_len; i++) {
        r_bits[i] = (rx[i] >= 0) ? 1 : 0;
      }

      rs_decode(r_bits, c_hat, u_hat);

      int info_err_bits = 0;
      for (int i = 0; i < info_bits_len; i++) {
        if (u_bits[i] != u_hat[i])
          info_err_bits++;
      }
      err_info += info_err_bits;

      int fer = (info_err_bits > 0);
      sum_frame_errors += fer;
    }

    double BER_RS = (double)err_info / (double)total_info_bits;
    double BER_BPSK = bpsk_ber(EbN0);
    double BLER_RS = (double)sum_frame_errors / (double)N_TRIALS;

    /* BPSK BLER 理論値 (符号語長 N*m ビット基準) */
    double BLER_BPSK = 1.0 - pow(1.0 - BER_BPSK, code_bits_len);

    printf("%4.1f, %.10e, %.10e, %.10e, %.10e\n", EbN0_dB, BER_RS, BER_BPSK,
           BLER_RS, BLER_BPSK);
    fprintf(fp, "%4.1f,%.10e,%.10e\n", EbN0_dB, BER_RS, BER_BPSK);
    fprintf(fp_bler, "%4.1f,%.10e,%.10e\n", EbN0_dB, BLER_RS, BLER_BPSK);
  }

  fclose(fp);
  fclose(fp_bler);

  free(u_bits);
  free(c_bits);
  free(r_bits);
  free(c_hat);
  free(u_hat);
  free(tx);
  free(rx);

  printf("\nResults saved to results/rs_ber_data.csv and "
         "results/rs_bler_data.csv\n");
  return 0;
}
