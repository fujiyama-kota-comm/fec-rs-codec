/**
 * @file rs_encoder.c
 * @brief Reed–Solomon encoder (systematic) using GF(2^m) arithmetic.
 *
 * This module implements a standard systematic RS encoder using the
 * generator polynomial g(x) constructed in rs_gf_init().
 *
 * Input:
 *   - inf_bits : K * m bits (information symbols)
 *
 * Output:
 *   - code_bits : (K + T) * m bits (systematic RS codeword)
 *
 * Encoding rule:
 *   Let the codeword be: [u0, u1, ..., u_(K-1), p0, ..., p_(T-1)]
 *
 *   Parity computation is implemented using the classical shift-register
 *   architecture:
 *       parity ← (parity << 1) ⊕ feedback * g(x)
 *   where g(x) is the generator polynomial.
 *
 * Shortened RS codes:
 *   When shortening is used, S = Np - N dummy symbols are shifted through
 *   the encoder before feeding the K actual symbols.
 */

#include "rs_encoder.h"
#include "rs_gf.h"
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Helpers: Conversion between bit arrays and GF symbols
 * ------------------------------------------------------------------------- */

/**
 * @brief Convert m bits → GF symbol (uint16_t).
 */
static uint16_t bits_to_symbol(const int *bits, int m) {
  uint16_t v = 0;
  for (int i = 0; i < m; i++)
    v |= (bits[i] & 1) << i;
  return v;
}

/**
 * @brief Convert GF symbol → m bits.
 *
 * Uses precomputed rs_symbol_bits[] for speed.
 */
static void symbol_to_bits(uint16_t sym, int *bits, int m) {
  for (int b = 0; b < m; b++)
    bits[b] = rs_symbol_bits[sym][b];
}

/* -------------------------------------------------------------------------
 * Systematic RS encoding
 * ------------------------------------------------------------------------- */

/**
 * @brief Systematic Reed–Solomon encoder.
 *
 * Produces a codeword of:
 *      [K info symbols][T parity symbols]
 *
 * @param inf_bits  Input bit array (K * m bits).
 * @param code_bits Output bit array ((K + T) * m bits).
 */
void rs_encode(const int *inf_bits, int *code_bits) {
  int m = rs_m;
  int K = rs_K;
  int T = rs_T;
  int S = rs_S;

  /* -------------------------------------------------------------
   * Convert K information symbols from bits → GF symbols
   * ------------------------------------------------------------- */
  uint16_t u[K];
  for (int i = 0; i < K; i++)
    u[i] = bits_to_symbol(&inf_bits[i * m], m);

  /* -------------------------------------------------------------
   * Initialize T parity registers to zero
   * ------------------------------------------------------------- */
  uint16_t parity[T];
  for (int i = 0; i < T; i++)
    parity[i] = 0;

  /* -------------------------------------------------------------
   * Handle shortening:
   *   Shift S dummy symbols (all zeros) through the encoder.
   *
   * This produces the same result as encoding an N-symbol RS code
   * and then shortening it to length N.
   * ------------------------------------------------------------- */
  for (int s = 0; s < S; s++) {
    uint16_t fb = parity[0];
    for (int j = 0; j < T - 1; j++)
      parity[j] = rs_gf_add(parity[j + 1], rs_gf_mul(fb, rs_generator[j + 1]));
    parity[T - 1] = rs_gf_mul(fb, rs_generator[T]);
  }

  /* -------------------------------------------------------------
   * Feed the actual K information symbols
   * ------------------------------------------------------------- */
  for (int i = 0; i < K; i++) {
    uint16_t fb = rs_gf_add(u[i], parity[0]);
    for (int j = 0; j < T - 1; j++)
      parity[j] = rs_gf_add(parity[j + 1], rs_gf_mul(fb, rs_generator[j + 1]));
    parity[T - 1] = rs_gf_mul(fb, rs_generator[T]);
  }

  /* -------------------------------------------------------------
   * Output systematic codeword:
   *     [ info symbols ][ parity symbols ]
   * Convert back to bits.
   * ------------------------------------------------------------- */
  for (int i = 0; i < K; i++)
    symbol_to_bits(u[i], &code_bits[i * m], m);

  for (int i = 0; i < T; i++)
    symbol_to_bits(parity[i], &code_bits[(K + i) * m], m);
}
