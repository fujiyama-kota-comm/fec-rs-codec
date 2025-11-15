#ifndef NSC_ENCODER_H
#define NSC_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 *  NSC Encoder (Non-Systematic Convolutional Code)
 *  ============================================================================
 *  ・レート 1/2 の非系統的畳み込み符号器（Constraint length = 3, 4 states）
 *  ・実装は trellis.h における状態遷移テーブル
 *        - nsc_next_state[s][b]
 *        - nsc_output_bits[s][b][2]
 *    を用いた「if 文なしのテーブル駆動型」方式
 *
 *  ・終端方式:
 *        tail bits = {0,0} を追加し、必ず STATE_A (00) に戻す
 *
 *  ・符号長:
 *        入力 K ビット → 出力 2*(K + tail_len) ビット
 *        （tail_len = 2 がデフォルト）
 *
 *  ・使用前に呼び出し側で必ず以下を設定すること:
 *        nsc_info_len  : 情報ビット長 K
 *        nsc_code_len  : 符号ビット長 N = 2*(K + tail_len)
 *        nsc_tail_len  : tail ビット数（通常 2）
 * ========================================================================== */

/* --- Global parameters (呼び出し側で設定する) ---------------------------- */
extern int nsc_info_len; /* 情報ビット長 K */
extern int nsc_code_len; /* 符号ビット長 N = 2*(K + tail_len) */
extern int nsc_tail_len; /* tail bits 数（通常 2） */

/* ============================================================================
 *  Function: nsc_encode_r05
 *  --------------------------------------------------------------------------
 *  畳み込み符号器（レート 1/2）
 *
 *  Parameters:
 *      data  : 入力情報ビット列 (長さ K)
 *      code  : 出力符号ビット列 (長さ 2*(K + tail_len))
 *
 *  Notes:
 *      - 終端 tail bits = {0,0} を内部で自動的に付加する。
 *      - trellis.h のテーブルに従い if 文無しで状態遷移を行う。
 * ========================================================================= */
void nsc_encode_r05(const int *data, int *code);

#ifdef __cplusplus
}
#endif

#endif /* NSC_ENCODER_H */
