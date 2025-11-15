#include "rs_encoder.h"
#include "rs_gf.h"
#include <stdint.h>

static uint16_t bits_to_symbol(const int *bits, int m) {
  uint16_t v = 0;
  for (int i = 0; i < m; i++)
    v |= (bits[i] & 1) << i;
  return v;
}
static void symbol_to_bits(uint16_t sym, int *bits, int m) {
  for (int b = 0; b < m; b++)
    bits[b] = rs_symbol_bits[sym][b];
}

void rs_encode(const int *inf_bits, int *code_bits) {
  int m = rs_m, K = rs_K, T = rs_T, S = rs_S;
  uint16_t u[K];
  for (int i = 0; i < K; i++)
    u[i] = bits_to_symbol(&inf_bits[i * m], m);
  uint16_t parity[T];
  for (int i = 0; i < T; i++)
    parity[i] = 0;
  for (int s = 0; s < S; s++) {
    uint16_t fb = parity[0];
    for (int j = 0; j < T - 1; j++)
      parity[j] = rs_gf_add(parity[j + 1], rs_gf_mul(fb, rs_generator[j + 1]));
    parity[T - 1] = rs_gf_mul(fb, rs_generator[T]);
  }
  for (int i = 0; i < K; i++) {
    uint16_t fb = rs_gf_add(u[i], parity[0]);
    for (int j = 0; j < T - 1; j++)
      parity[j] = rs_gf_add(parity[j + 1], rs_gf_mul(fb, rs_generator[j + 1]));
    parity[T - 1] = rs_gf_mul(fb, rs_generator[T]);
  }
  for (int i = 0; i < K; i++)
    symbol_to_bits(u[i], &code_bits[i * m], m);
  for (int i = 0; i < T; i++)
    symbol_to_bits(parity[i], &code_bits[(K + i) * m], m);
}
