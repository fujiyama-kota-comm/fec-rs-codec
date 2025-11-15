#include "rs_gf.h"
#include <stdio.h>
#include <stdlib.h>

int rs_m = 0, rs_N = 0, rs_Np = 0, rs_S = 0, rs_K = 0, rs_T = 0;
uint16_t rs_gf_exp[2 * RS_GF_MAX];
uint16_t rs_gf_log[RS_GF_MAX];
uint16_t rs_generator[RS_GF_MAX];
int rs_symbol_bits[RS_GF_MAX][RS_M_MAX];

static const uint16_t primitive_poly[9] = {0x00, 0x03, 0x07, 0x0B, 0x13,
                                           0x25, 0x43, 0x89, 0x11D};

uint16_t rs_gf_add(uint16_t a, uint16_t b) { return a ^ b; }
uint16_t rs_gf_mul(uint16_t a, uint16_t b) {
  if (a == 0 || b == 0)
    return 0;
  int idx = rs_gf_log[a] + rs_gf_log[b];
  if (idx >= rs_Np)
    idx -= rs_Np;
  return rs_gf_exp[idx];
}
uint16_t rs_gf_div(uint16_t a, uint16_t b) {
  if (b == 0) {
    fprintf(stderr, "div0\n");
    exit(1);
  }
  if (a == 0)
    return 0;
  int idx = rs_gf_log[a] - rs_gf_log[b];
  if (idx < 0)
    idx += rs_Np;
  return rs_gf_exp[idx];
}
uint16_t rs_gf_pow(uint16_t base, int power) {
  if (base == 0)
    return 0;
  int logv = rs_gf_log[base];
  int x = (logv * power) % rs_Np;
  if (x < 0)
    x += rs_Np;
  return rs_gf_exp[x];
}
uint16_t rs_gf_inv(uint16_t a) {
  if (a == 0)
    return 0;
  return rs_gf_exp[rs_Np - rs_gf_log[a]];
}

int rs_gf_init(int m, int N, int K, int T) {
  rs_m = m;
  rs_N = N;
  rs_K = K;
  rs_T = T;
  rs_Np = (1 << m) - 1;
  rs_S = rs_Np - rs_N;
  if (rs_S < 0) {
    fprintf(stderr, "N too large\n");
    return -1;
  }
  uint16_t prim = primitive_poly[m];
  uint16_t x = 1;
  for (int i = 0; i < rs_Np; i++) {
    rs_gf_exp[i] = x;
    rs_gf_log[x] = i;
    x <<= 1;
    if (x & (1u << m))
      x ^= prim;
  }
  for (int i = rs_Np; i < 2 * rs_Np; i++)
    rs_gf_exp[i] = rs_gf_exp[i - rs_Np];
  rs_gf_log[0] = 0;
  for (int i = 0; i <= T; i++)
    rs_generator[i] = 0;
  rs_generator[0] = 1;
  uint16_t tmp[RS_GF_MAX];
  for (int i = 0; i < T; i++) {
    for (int j = 0; j <= i; j++)
      tmp[j] = rs_generator[j];
    rs_generator[i + 1] = 0;
    for (int j = i + 1; j >= 1; j--) {
      uint16_t term_const = (j <= i) ? rs_gf_mul(tmp[j], rs_gf_exp[i]) : 0;
      rs_generator[j] = rs_gf_add(tmp[j - 1], term_const);
    }
    rs_generator[0] = rs_gf_mul(tmp[0], rs_gf_exp[i]);
  }
  uint16_t g0 = rs_generator[0];
  uint16_t inv_g0 = rs_gf_inv(g0);
  for (int j = 0; j <= T; j++)
    rs_generator[j] = rs_gf_mul(rs_generator[j], inv_g0);
  int max_val = 1 << m;
  for (int val = 0; val < max_val; val++) {
    for (int b = 0; b < m; b++)
      rs_symbol_bits[val][b] = (val >> b) & 1;
    for (int b = m; b < RS_M_MAX; b++)
      rs_symbol_bits[val][b] = 0;
  }
  return 0;
}
