#ifndef RS_GF_H
#define RS_GF_H

#include <stdint.h>

#define RS_M_MAX 8
#define RS_GF_MAX (1 << RS_M_MAX) /* 256 */

extern int rs_m;  /* GF(2^m) */
extern int rs_N;  /* shortened length Ns */
extern int rs_Np; /* parent length Np = 2^m - 1 */
extern int rs_S;  /* shortening amount */
extern int rs_K;  /* info length */
extern int rs_T;  /* parity length */

extern uint16_t rs_gf_exp[2 * RS_GF_MAX];
extern uint16_t rs_gf_log[RS_GF_MAX];
extern uint16_t rs_generator[RS_GF_MAX];
extern int rs_symbol_bits[RS_GF_MAX][RS_M_MAX];

uint16_t rs_gf_add(uint16_t a, uint16_t b);
uint16_t rs_gf_mul(uint16_t a, uint16_t b);
uint16_t rs_gf_div(uint16_t a, uint16_t b);
uint16_t rs_gf_pow(uint16_t base, int power);
uint16_t rs_gf_inv(uint16_t a);

int rs_gf_init(int m, int N, int K, int T);

#endif
