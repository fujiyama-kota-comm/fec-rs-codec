/**
 * @file rs_gf.h
 * @brief Finite Field (GF(2^m)) routines for Reed–Solomon encoding/decoding.
 *
 * This header declares:
 *   - Global RS parameters (m, N, K, T)
 *   - GF(2^m) tables (exp/log)
 *   - Generator polynomial storage
 *   - Per-symbol bit decomposition table
 *   - Basic GF arithmetic functions
 *   - Initialization routine for generating all tables
 *
 * All implementation details are in rs_gf.c.
 */

#ifndef RS_GF_H
#define RS_GF_H

#include <stdint.h>

/* -------------------------------------------------------------------------
 * Configuration
 * ------------------------------------------------------------------------- */
#define RS_M_MAX 8
#define RS_GF_MAX (1 << RS_M_MAX) /* Maximum GF size = 2^8 = 256 */

/* -------------------------------------------------------------------------
 * Global Reed–Solomon parameters (set by rs_gf_init)
 * ------------------------------------------------------------------------- */
extern int rs_m;  /* GF size parameter m → GF(2^m) */
extern int rs_N;  /* Codeword length (shortened) */
extern int rs_Np; /* Parent GF length = 2^m - 1 */
extern int rs_S;  /* Shortening amount = Np - N */
extern int rs_K;  /* Number of information symbols */
extern int rs_T;  /* Number of parity symbols (generator degree) */

/* -------------------------------------------------------------------------
 * GF tables and polynomial data
 * ------------------------------------------------------------------------- */
extern uint16_t rs_gf_exp[2 * RS_GF_MAX];       /* Exponential table */
extern uint16_t rs_gf_log[RS_GF_MAX];           /* Logarithm table */
extern uint16_t rs_generator[RS_GF_MAX];        /* Generator polynomial g(x) */
extern int rs_symbol_bits[RS_GF_MAX][RS_M_MAX]; /* Bit representation table */

/* -------------------------------------------------------------------------
 * GF(2^m) arithmetic primitives
 * ------------------------------------------------------------------------- */

/**
 * @brief GF addition (same as XOR).
 */
uint16_t rs_gf_add(uint16_t a, uint16_t b);

/**
 * @brief GF multiplication using exp/log tables.
 */
uint16_t rs_gf_mul(uint16_t a, uint16_t b);

/**
 * @brief GF division using exp/log tables.
 */
uint16_t rs_gf_div(uint16_t a, uint16_t b);

/**
 * @brief Raise base to an integer power (base^power) in GF.
 */
uint16_t rs_gf_pow(uint16_t base, int power);

/**
 * @brief Multiplicative inverse in GF.
 */
uint16_t rs_gf_inv(uint16_t a);

/* -------------------------------------------------------------------------
 * Initialization
 * ------------------------------------------------------------------------- */

/**
 * @brief Initialize GF(2^m) and construct RS generator polynomial.
 *
 * @param m  GF size parameter (1–8), GF size = 2^m
 * @param N  Codeword length (shortened)
 * @param K  Information symbol length
 * @param T  Parity symbol length (degree of generator polynomial)
 *
 * @return 0 on success, negative on failure.
 */
int rs_gf_init(int m, int N, int K, int T);

#endif /* RS_GF_H */
