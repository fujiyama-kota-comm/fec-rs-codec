/**
 * @file rs_decoder.c
 * @brief Reed–Solomon decoder (shortened, systematic) for GF(2^m).
 *
 * This module implements the classical RS decoding pipeline:
 *
 *    1) Syndrome computation
 *    2) Berlekamp–Massey algorithm (error-locator polynomial σ(x))
 *    3) Chien search (find error positions)
 *    4) Solve error magnitudes (simple Forney via linear system)
 *    5) Apply corrections and return:
 *          - Corrected codeword (Ns symbols)
 *          - Decoded information symbols (first K symbols)
 *
 * Shortening:
 *   A shortened RS code is handled by conceptually padding the front
 *   with S = Np - Ns zero-symbols, performing full decoding on Np,
 *   and then dropping the first S parent-symbols.
 *
 * The decoder assumes:
 *   - rs_gf_init() has been called
 *   - recv_bits contains Ns * m bits
 */

#include "rs_decoder.h"
#include "rs_gf.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Helpers: bits <-> symbol (LSB-first ordering)
 * ------------------------------------------------------------------------- */
static uint16_t bits_to_symbol(const int *bits, int m) {
  uint16_t v = 0;
  for (int i = 0; i < m; i++)
    v |= (uint16_t)((bits[i] & 1) << i);
  return v;
}

static void symbol_to_bits(uint16_t symbol, int *bits, int m) {
  for (int b = 0; b < m; b++)
    bits[b] = rs_symbol_bits[symbol][b];
}

/* -------------------------------------------------------------------------
 * 1) Syndrome computation (on parent length Np)
 *
 *     S_i = Σ_{j=0}^{Np-1} r_j α^{(i+1)*j},   for i = 0..T-1
 *
 * Zero syndromes → no errors.
 * ------------------------------------------------------------------------- */
static void compute_syndromes(const uint16_t *recv_sym_p, uint16_t *S) {
  int Np = rs_Np;
  int T = rs_T;

  for (int i = 0; i < T; i++) {
    uint16_t sum = 0;
    int si = i + 1; /* Evaluate at α^(i+1) */

    for (int j = 0; j < Np; j++) {
      int k = (si * j) % rs_Np;
      sum ^= rs_gf_mul(recv_sym_p[j], rs_gf_exp[k]);
    }
    S[i] = sum;
  }
}

/* -------------------------------------------------------------------------
 * 2) Berlekamp–Massey algorithm
 *
 * Finds the error-locator polynomial σ(x).
 * Output: sigma_out[0..L]
 * Ensures σ(0) = 1.
 *
 * L = degree of error-locator polynomial
 * ------------------------------------------------------------------------- */
static int berlekamp_massey(const uint16_t *S, uint16_t *sigma_out) {
  int T = rs_T;
  int t = T / 2;

  uint16_t C[RS_GF_MAX] = {0}; /* current polynomial */
  uint16_t B[RS_GF_MAX] = {0}; /* previous polynomial */
  C[0] = 1;
  B[0] = 1;

  int L = 0;
  int m_shift = 1;
  uint16_t bbb = 1;

  for (int n = 0; n < T; n++) {
    uint16_t d = S[n];
    for (int i = 1; i <= L; i++)
      d ^= rs_gf_mul(C[i], S[n - i]);

    if (d != 0) {
      uint16_t Temp[RS_GF_MAX];
      for (int i = 0; i <= T; i++)
        Temp[i] = C[i];

      uint16_t coef = rs_gf_div(d, bbb);

      /* C(x) ← C(x) - coef * x^m_shift * B(x) */
      for (int i = 0; i <= T; i++) {
        int idx = i + m_shift;
        if (idx <= T)
          C[idx] ^= rs_gf_mul(coef, B[i]);
      }

      if (2 * L <= n) {
        /* Update B(x) ← previous C(x) */
        for (int i = 0; i <= T; i++)
          B[i] = Temp[i];
        L = n + 1 - L;
        bbb = d;
        m_shift = 1;
      } else {
        m_shift++;
      }
    } else {
      m_shift++;
    }
  }

  /* Copy result */
  for (int i = 0; i <= t; i++)
    sigma_out[i] = 0;
  for (int i = 0; i <= L && i <= t; i++)
    sigma_out[i] = C[i];

  /* Ensure σ(0) = 1 */
  if (sigma_out[0] == 0)
    sigma_out[0] = 1;

  return L;
}

/* -------------------------------------------------------------------------
 * 3) Chien search
 *
 * Find i such that σ(α^{-i}) = 0, for i = 0..Np-1.
 * Each such i corresponds to an error at position i.
 * ------------------------------------------------------------------------- */
static int chien_search(const uint16_t *sigma, int L, int *error_pos) {
  int Np = rs_Np;
  int count = 0;

  for (int i = 0; i < Np; i++) {
    uint16_t x_inv = (i == 0) ? 1 : rs_gf_exp[rs_Np - i];
    uint16_t sum = 0;
    uint16_t power = 1;

    for (int j = 0; j <= L; j++) {
      if (sigma[j] != 0)
        sum ^= rs_gf_mul(sigma[j], power);
      power = rs_gf_mul(power, x_inv);
    }

    if (sum == 0)
      error_pos[count++] = i;

    if (count > L)
      break;
  }

  return count;
}

/* -------------------------------------------------------------------------
 * 4) Error magnitude solving via linear system
 *
 * Simplified Forney method:
 *     S_l = Σ e_k α^{(l+1) * i_k}
 * Solve for e_k using Gaussian elimination in GF(2^m).
 * ------------------------------------------------------------------------- */
static void correct_errors(uint16_t *recv_sym_p, const uint16_t *S,
                           const int *error_pos, int error_count) {
  if (error_count <= 0)
    return;

  int cnt = error_count;
  int Np = rs_Np;

  uint16_t A[cnt][cnt];
  uint16_t B[cnt];
  uint16_t e[cnt];

  /* Construct linear system */
  for (int r = 0; r < cnt; r++) {
    B[r] = S[r];
    for (int c = 0; c < cnt; c++) {
      int pos = error_pos[c];
      int exp = ((r + 1) * pos) % Np;
      A[r][c] = rs_gf_exp[exp];
    }
  }

  /* Gaussian elimination over GF(2^m) */
  for (int i = 0; i < cnt; i++) {
    uint16_t piv = A[i][i];

    if (piv == 0) {
      int swap_r = -1;
      for (int r = i + 1; r < cnt; r++)
        if (A[r][i] != 0) {
          swap_r = r;
          break;
        }

      if (swap_r >= 0) {
        for (int c = 0; c < cnt; c++) {
          uint16_t tmp = A[i][c];
          A[i][c] = A[swap_r][c];
          A[swap_r][c] = tmp;
        }
        uint16_t tmpB = B[i];
        B[i] = B[swap_r];
        B[swap_r] = tmpB;
        piv = A[i][i];
      }
    }

    if (piv == 0)
      continue;

    uint16_t inv = rs_gf_inv(piv);
    for (int c = 0; c < cnt; c++)
      A[i][c] = rs_gf_mul(A[i][c], inv);
    B[i] = rs_gf_mul(B[i], inv);

    for (int r = 0; r < cnt; r++) {
      if (r == i)
        continue;
      uint16_t factor = A[r][i];
      if (factor == 0)
        continue;

      for (int c = 0; c < cnt; c++)
        A[r][c] = rs_gf_add(A[r][c], rs_gf_mul(factor, A[i][c]));
      B[r] = rs_gf_add(B[r], rs_gf_mul(factor, B[i]));
    }
  }

  for (int i = 0; i < cnt; i++)
    e[i] = B[i];

  /* Apply error corrections */
  for (int k = 0; k < cnt; k++) {
    int pos = error_pos[k];
    recv_sym_p[pos] ^= e[k];
  }
}

/* -------------------------------------------------------------------------
 * 5) Public API: RS decoding
 *
 * Steps:
 *   - Expand to parent length: [S zero-symbols][Ns received]
 *   - Compute syndromes
 *   - If non-zero: BM → Chien → Solve magnitudes → Correct
 *   - Output:
 *       code_bits : Ns symbols
 *       info_bits : first K symbols
 * ------------------------------------------------------------------------- */
void rs_decode(const int *recv_bits, int *code_bits, int *info_bits) {
  int m = rs_m;
  int Ns = rs_N;
  int Np = rs_Np;
  int S = rs_S;
  int K = rs_K;
  int T = rs_T;
  int t = T / 2;

  /* Build parent-length buffer */
  uint16_t recv_sym_p[Np];

  for (int i = 0; i < S; i++)
    recv_sym_p[i] = 0;

  for (int i = 0; i < Ns; i++)
    recv_sym_p[S + i] = bits_to_symbol(&recv_bits[i * m], m);

  /* Syndromes */
  uint16_t synd[T];
  compute_syndromes(recv_sym_p, synd);

  /* Check if all-zero syndromes → no errors */
  int all_zero = 1;
  for (int i = 0; i < T; i++)
    if (synd[i] != 0) {
      all_zero = 0;
      break;
    }

  if (!all_zero) {
    /* BM → locator polynomial */
    uint16_t sigma[t + 1];
    int L = berlekamp_massey(synd, sigma);
    if (L > t)
      L = t;

    /* Chien search */
    int error_pos[t];
    int count = chien_search(sigma, L, error_pos);

    /* Correct */
    if (count > 0 && count <= t)
      correct_errors(recv_sym_p, synd, error_pos, count);
  }

  /* Output corrected shortened codeword */
  for (int i = 0; i < Ns; i++)
    symbol_to_bits(recv_sym_p[S + i], &code_bits[i * m], m);

  /* Output K information symbols */
  for (int i = 0; i < K; i++)
    symbol_to_bits(recv_sym_p[S + i], &info_bits[i * m], m);
}
