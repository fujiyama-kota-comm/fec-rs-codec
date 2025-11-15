/**
 * @file rs_gf.c
 * @brief Galois Field (GF(2^m)) arithmetic for Reed–Solomon codes.
 *
 * This module initializes the finite field GF(2^m), manages exponential/log
 * tables, provides basic operations (add/mul/div/inv/pow), and generates the
 * RS generator polynomial of degree T.
 *
 * Supported:
 *   - GF sizes up to RS_GF_MAX (configurable in rs_gf.h)
 *   - Arbitrary (N, K, T) compatible with GF(2^m)
 *
 * This implementation is self-contained and uses only standard C.
 */

#include "rs_gf.h"
#include <stdio.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * Global RS parameters (set by rs_gf_init)
 * ------------------------------------------------------------------------- */
int rs_m = 0, rs_N = 0, rs_Np = 0, rs_S = 0, rs_K = 0, rs_T = 0;

/* Exponential/log tables for GF(2^m) */
uint16_t rs_gf_exp[2 * RS_GF_MAX];
uint16_t rs_gf_log[RS_GF_MAX];

/* Generator polynomial coefficients g(x) (degree T) */
uint16_t rs_generator[RS_GF_MAX];

/* Lookup table: symbol (0..2^m-1) → per-bit representation */
int rs_symbol_bits[RS_GF_MAX][RS_M_MAX];

/* Primitive polynomials for m = 1..8 (CCSDS/NASA compatible) */
static const uint16_t primitive_poly[9] = {
    0x00, /* unused (m=0) */
    0x03, /* m=1 */
    0x07, /* m=2 */
    0x0B, /* m=3 */
    0x13, /* m=4 */
    0x25, /* m=5 */
    0x43, /* m=6 */
    0x89, /* m=7 */
    0x11D /* m=8 (used for RS(255,223), GF(256)) */
};

/* -------------------------------------------------------------------------
 * Basic GF operations
 * ------------------------------------------------------------------------- */

/** GF addition = XOR */
uint16_t rs_gf_add(uint16_t a, uint16_t b) { return a ^ b; }

/** GF multiplication using log/exp tables */
uint16_t rs_gf_mul(uint16_t a, uint16_t b) {
  if (a == 0 || b == 0)
    return 0;
  int idx = rs_gf_log[a] + rs_gf_log[b];
  if (idx >= rs_Np)
    idx -= rs_Np;
  return rs_gf_exp[idx];
}

/** GF division using log/exp tables */
uint16_t rs_gf_div(uint16_t a, uint16_t b) {
  if (b == 0) {
    fprintf(stderr, "ERROR: GF division by zero\n");
    exit(1);
  }
  if (a == 0)
    return 0;
  int idx = rs_gf_log[a] - rs_gf_log[b];
  if (idx < 0)
    idx += rs_Np;
  return rs_gf_exp[idx];
}

/** GF exponentiation (base^power in GF) */
uint16_t rs_gf_pow(uint16_t base, int power) {
  if (base == 0)
    return 0;
  int logv = rs_gf_log[base];
  int x = (logv * power) % rs_Np;
  if (x < 0)
    x += rs_Np;
  return rs_gf_exp[x];
}

/** Multiplicative inverse */
uint16_t rs_gf_inv(uint16_t a) {
  if (a == 0)
    return 0;
  return rs_gf_exp[rs_Np - rs_gf_log[a]];
}

/* -------------------------------------------------------------------------
 * Initialize GF(2^m) and build generator polynomial g(x)
 * ------------------------------------------------------------------------- */
int rs_gf_init(int m, int N, int K, int T) {
  rs_m = m;
  rs_N = N;
  rs_K = K;
  rs_T = T;

  /* Field size (2^m - 1) */
  rs_Np = (1 << m) - 1;

  /* Number of shortened symbols */
  rs_S = rs_Np - rs_N;
  if (rs_S < 0) {
    fprintf(stderr, "ERROR: N exceeds field maximum (2^m - 1)\n");
    return -1;
  }

  /* Select primitive polynomial */
  uint16_t prim = primitive_poly[m];

  /* Build exp/log tables */
  uint16_t x = 1;
  for (int i = 0; i < rs_Np; i++) {
    rs_gf_exp[i] = x;
    rs_gf_log[x] = i;

    x <<= 1;
    if (x & (1u << m))
      x ^= prim;
  }

  /* Extend exp table for mod-free multiplication */
  for (int i = rs_Np; i < 2 * rs_Np; i++)
    rs_gf_exp[i] = rs_gf_exp[i - rs_Np];

  rs_gf_log[0] = 0;

  /* ---------------------------------------------------------------------
   * Generator polynomial construction (degree T)
   * g(x) = (x - α^0)(x - α^1)...(x - α^(T-1))
   * --------------------------------------------------------------------- */
  for (int i = 0; i <= T; i++)
    rs_generator[i] = 0;
  rs_generator[0] = 1;

  uint16_t tmp[RS_GF_MAX];

  for (int i = 0; i < T; i++) {
    /* Copy existing coefficients */
    for (int j = 0; j <= i; j++)
      tmp[j] = rs_generator[j];

    rs_generator[i + 1] = 0;

    /* Perform polynomial multiplication by (x - α^i) */
    for (int j = i + 1; j >= 1; j--) {
      uint16_t term = (j <= i) ? rs_gf_mul(tmp[j], rs_gf_exp[i]) : 0;
      rs_generator[j] = rs_gf_add(tmp[j - 1], term);
    }
    rs_generator[0] = rs_gf_mul(tmp[0], rs_gf_exp[i]);
  }

  /* Normalize g(x) so that g[0] = 1 */
  uint16_t g0 = rs_generator[0];
  uint16_t inv_g0 = rs_gf_inv(g0);
  for (int j = 0; j <= T; j++)
    rs_generator[j] = rs_gf_mul(rs_generator[j], inv_g0);

  /* ---------------------------------------------------------------------
   * Precompute symbol bit-representation table
   * --------------------------------------------------------------------- */
  int max_val = 1 << m;
  for (int val = 0; val < max_val; val++) {
    for (int b = 0; b < m; b++)
      rs_symbol_bits[val][b] = (val >> b) & 1;
    for (int b = m; b < RS_M_MAX; b++)
      rs_symbol_bits[val][b] = 0;
  }

  return 0;
}
