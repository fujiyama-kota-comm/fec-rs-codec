#ifndef TRELLIS_H
#define TRELLIS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 *  Trellis definition for NSC (Non-Systematic Convolutional Code)
 *  ============================================================================
 *  ・2-bit shift register を持つ 4 状態の NSC 符号用 trellis。
 *  ・符号器/復号器は、このヘッダで定義される
 *        - NSCState 列挙体（状態ラベル）
 *        - nsc_output_bits[state][input_bit][2]
 *        - nsc_next_state [state][input_bit]
 *    にのみ依存するテーブル駆動型実装となる。
 *
 *  状態は 2-bit レジスタ内容に対応し、以下の 4 通り:
 *
 *      STATE_A : 00
 *      STATE_B : 01
 *      STATE_C : 10
 *      STATE_D : 11
 *
 *  入力ビット b ∈ {0,1} に対して
 *
 *      - 出力ビット列 (v, w)
 *      - 次状態 next_state
 *
 *  を trellis.c 側で定義されたテーブルから参照する。
 * ========================================================================== */

/* --------------------------------------------------------------------------
 *  4 状態定義（2-bit shift register の内容に対応）
 * ------------------------------------------------------------------------ */
typedef enum {
  STATE_A = 0, /* shift register = 00 */
  STATE_B = 1, /* shift register = 01 */
  STATE_C = 2, /* shift register = 10 */
  STATE_D = 3  /* shift register = 11 */
} NSCState;

/* --------------------------------------------------------------------------
 *  出力ビット表 nsc_output_bits[state][input][2]
 *  ------------------------------------------------------------------------
 *  意味:
 *      nsc_output_bits[s][b][0] : MSB 側の出力ビット v
 *      nsc_output_bits[s][b][1] : LSB 側の出力ビット w
 *
 *  例:
 *      const int (*tbl)[2][2] = nsc_output_bits;
 *      v = tbl[STATE_A][1][0];   // state=A, input=1 のときの v
 *      w = tbl[STATE_A][1][1];   // 同上の w
 * ------------------------------------------------------------------------ */
extern const int nsc_output_bits[4][2][2];

/* --------------------------------------------------------------------------
 *  次状態表 nsc_next_state[state][input]
 *  ------------------------------------------------------------------------
 *  意味:
 *      nsc_next_state[s][b] : 現状態 s で入力ビット b を入れたときの次状態
 *
 *  例:
 *      state = nsc_next_state[state][input_bit];
 * ------------------------------------------------------------------------ */
extern const int nsc_next_state[4][2];

#ifdef __cplusplus
}
#endif

#endif /* TRELLIS_H */
