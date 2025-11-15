#include "nsc_encoder.h"
#include "trellis.h"
#include <stdio.h>
#include <stdlib.h>

/* ========================================================================
 *  Non-Systematic Convolutional (NSC) Encoder
 *  ----------------------------------------------------------------------
 *  ・レート 1/2 の非系統的畳み込み符号（NASA 率の一般的な構造）
 *  ・符号器は trellis.h の
 *        - nsc_next_state[s][b]   : 次状態
 *        - nsc_output_bits[s][b]  : 出力系列 (2bit)
 *    に完全依存する「if 分岐の無い」テーブル駆動型実装
 *
 *  ・内部状態数 : 4 (2-bit shift register)
 *        STATE_A = 00, STATE_B = 01, STATE_C = 10, STATE_D = 11
 *
 *  ・終端方式:
 *        tail bits = {0, 0} を追加し、終端状態を強制的に STATE_A へ戻す
 *
 *  ・code 長:
 *        N = 2 * (K + tail_len)
 *        ※ レート 1/2 のため、1 入力ビット → 2 出力ビット
 *
 *  注意:
 *        グローバル変数 nsc_info_len / nsc_code_len は
 *        呼び出し側（main など）で必ず設定しておくこと。
 * ======================================================================== */

/* --- グローバル変数（定義はここ、宣言はヘッダ側） --------------------- */
int nsc_info_len;     // 情報ビット長 K
int nsc_code_len;     // 出力符号長 N = 2 * (K + nsc_tail_len)
int nsc_tail_len = 2; // 終端用テールビット数（2ビット固定）

/* ========================================================================
 *  NSC Encoder (Rate 1/2)
 *  ----------------------------------------------------------------------
 *  data : 入力ビット系列 (長さ K)
 *  code : 出力符号系列 (長さ N = 2*(K + 2))
 *
 *  説明:
 *    1) data[0..K-1] を buffer にコピー
 *    2) tail bits = {0,0} を付加し、必ず STATE_A に戻す
 *    3) Trellis のテーブル nsc_output_bits / nsc_next_state に従って
 *       if 分岐無しで符号化を実行
 *
 *  本関数は符号化のみを行い、データ長チェックは行わない。
 * ======================================================================== */
void nsc_encode_r05(const int *data, int *code) {

  int total_bits = nsc_info_len + nsc_tail_len; // 入力 + tail の総数

  /* ------------------------------------------------------------
   * 入力バッファ（情報ビット + tail）
   * ------------------------------------------------------------ */
  int *buffer = malloc(sizeof(int) * total_bits);
  if (!buffer) {
    fprintf(stderr, "[NSC Encoder] Memory allocation failed\n");
    return;
  }

  /* 情報ビットをコピー */
  for (int i = 0; i < nsc_info_len; i++) {
    buffer[i] = data[i];
  }

  /* 終端 tail bits = {0,0}
   * 畳み込み符号を確実に初期状態（STATE_A=00）へ戻すための標準手法。
   */
  buffer[nsc_info_len] = 0;
  buffer[nsc_info_len + 1] = 0;

  /* 初期状態は常に A (00) */
  int state = STATE_A;

  /* ------------------------------------------------------------
   * Trellis に従って遷移
   * 出力ビット:
   *      code[2*i], code[2*i+1]
   *
   * state 遷移:
   *      state = nsc_next_state[state][input_bit]
   *
   * 全てテーブル参照のみで、if 文による分岐は一切無し。
   * ------------------------------------------------------------ */
  for (int i = 0; i < total_bits; i++) {

    int b = buffer[i]; // 入力ビット (0 or 1)

    /* 出力 2bit を符号系列へ書き込み */
    code[2 * i] = nsc_output_bits[state][b][0];
    code[2 * i + 1] = nsc_output_bits[state][b][1];

    /* 次の状態へ */
    state = nsc_next_state[state][b];
  }

  free(buffer);
}
