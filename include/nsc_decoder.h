#ifndef NSC_DECODER_H
#define NSC_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 *  NSC Decoder (Non-Systematic Convolutional Code)
 *  ============================================================================
 *  ・レート 1/2 の非系統的畳み込み符号 (NSC) に対する Viterbi 復号器
 *  ・終端付き (tail bits により開始/終了状態を STATE_A に固定)
 *
 *  対応モード:
 *      (1) Soft-decision Viterbi  : LLR 入力 (double)
 *      (2) Hard-decision Viterbi  : 0/1 入力 (int)
 *
 *  入出力長:
 *      - 情報ビット長: K = nsc_info_len
 *      - 符号化ビット長: N = nsc_code_len = 2 * (K + tail_len)
 *        （tail_len = 2 が標準）
 *      - 復号側では N/2 = K + tail_len ステップを処理し，
 *        上位 K ステップのみ info_hat[] に書き込む。
 *
 *  注意:
 *      - nsc_info_len / nsc_code_len / nsc_tail_len は、
 *        事前に呼び出し側で設定しておく必要がある。
 *      - code_hat を NULL にした場合、再符号化は行わない。
 * ========================================================================== */

/* ============================================================================
 *  Function: nsc_decode_r05_soft
 *  --------------------------------------------------------------------------
 *  Soft-decision Viterbi decoder (LLR 入力)
 *
 *  Parameters:
 *      LLR      : 長さ N の LLR 列 (double)
 *      info_hat : 出力情報ビット列（長さ K）
 *      code_hat : 再符号化結果（任意, NULL の場合は未使用）
 *
 *  説明:
 *      - ブランチメトリックとして「負の内積」型を使用（BPSK LLR を想定）。
 *      - LLR に基づく最大尤度パスを探索する。
 * ========================================================================== */
void nsc_decode_r05_soft(const double *LLR, int *info_hat, int *code_hat);

/* ============================================================================
 *  Function: nsc_decode_r05_hard
 *  --------------------------------------------------------------------------
 *  Hard-decision Viterbi decoder (0/1 入力)
 *
 *  Parameters:
 *      rx_bits  : 長さ N の受信ビット列（0/1）
 *      info_hat : 出力情報ビット列（長さ K）
 *      code_hat : 再符号化結果（任意, NULL の場合は未使用）
 *
 *  説明:
 *      - ブランチメトリックとして Hamming 距離 (0,1,2) を使用。
 *      - Soft-decision 版と処理手順は同じだが、計算量が軽い。
 * ========================================================================== */
void nsc_decode_r05_hard(const int *rx_bits, int *info_hat, int *code_hat);

#ifdef __cplusplus
}
#endif

#endif /* NSC_DECODER_H */
