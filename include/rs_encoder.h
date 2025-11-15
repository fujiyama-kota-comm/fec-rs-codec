/**
 * @file rs_encoder.h
 * @brief Systematic Reed–Solomon encoder interface.
 *
 * This header declares the API for generating Reed–Solomon codewords
 * using the GF(2^m) arithmetic and generator polynomial initialized
 * in rs_gf_init().
 *
 * The encoder produces a systematic codeword:
 *     [ K information symbols ][ T parity symbols ]
 *
 * Bit representation:
 *   - Input  : K * m bits  → interpreted as K GF(2^m) symbols
 *   - Output : (K + T) * m bits (systematic RS codeword)
 *
 * Usage:
 *   1. Call rs_gf_init(m, N, K, T) once.
 *   2. Prepare an array of K*m bits.
 *   3. Call rs_encode() to obtain (K+T)*m bits.
 */

#ifndef RS_ENCODER_H
#define RS_ENCODER_H

/**
 * @brief Systematic Reed–Solomon encoding.
 *
 * @param inf_bits  Input information bits (array size = K * m).
 * @param code_bits Output codeword bits (array size = (K + T) * m).
 *
 * Requirements:
 *   - rs_gf_init() must be called before using this function.
 *   - inf_bits and code_bits must be valid, non-overlapping buffers.
 */
void rs_encode(const int *inf_bits, int *code_bits);

#endif /* RS_ENCODER_H */
