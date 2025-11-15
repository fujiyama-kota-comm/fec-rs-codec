/**
 * @file rs_decoder.h
 * @brief Reed–Solomon decoder interface (shortened, systematic).
 *
 * This header declares the API for RS decoding on GF(2^m).
 * It recovers the original information symbols as well as the corrected
 * shortened codeword.
 *
 * Decoding process (implemented in rs_decoder.c):
 *   1. Build parent-length input: [S zero-symbols][Ns received symbols]
 *   2. Compute syndromes
 *   3. Berlekamp–Massey → error-locator polynomial
 *   4. Chien search → error positions
 *   5. Solve error magnitudes → apply corrections
 *   6. Output:
 *        - code_bits : corrected shortened codeword (Ns symbols)
 *        - info_bits : first K symbols (decoded information)
 *
 * Requirements:
 *   - rs_gf_init(m, N, K, T) must be called before using this decoder.
 *   - recv_bits must have Ns * m elements.
 */

#ifndef RS_DECODER_H
#define RS_DECODER_H

/**
 * @brief Decode a shortened systematic Reed–Solomon codeword.
 *
 * @param recv_bits Input  received bits  (Ns * m bits).
 * @param code_bits Output corrected codeword bits (Ns * m bits).
 * @param info_bits Output decoded information bits (K * m bits).
 *
 * Notes:
 *   - This function performs full RS error correction.
 *   - Outputs are given in bit form (LSB-first ordering per symbol).
 */
void rs_decode(const int *recv_bits, int *code_bits, int *info_bits);

#endif /* RS_DECODER_H */
