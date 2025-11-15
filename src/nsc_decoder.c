#include "nsc_decoder.h"
#include "nsc_encoder.h"
#include "trellis.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* ========================================================================
 *  NSC Decoder (Rate 1/2, terminated)
 *  ----------------------------------------------------------------------
 *  ・レート 1/2 の非系統的畳み込み符号 (NSC) に対する Viterbi 復号器
 *  ・終端付き (tail bits により開始/終了状態を STATE_A に固定)
 *
 *  実装:
 *    - Soft-decision Viterbi (LLR 入力)
 *    - Hard-decision Viterbi (0/1 入力)
 *    - 全て trellis.h の
 *          nsc_next_state[s][b]      : 次状態
 *          nsc_output_bits[s][b][2]  : 出力 2bit
 *      に依存する、標準的な trellis 駆動型 Viterbi アルゴリズム。
 *
 *  入出力長:
 *    - 情報ビット長: K = nsc_info_len
 *    - 符号ビット長: N = nsc_code_len = 2 * (K + tail_len)
 *      （tail_len = 2 のとき、シンボル数 steps = K + 2）
 *
 *  注意:
 *    - nsc_info_len / nsc_code_len は、使用前に呼び出し側で設定しておくこと。
 *    - 終端付きのため、「先頭状態」「終端状態」ともに STATE_A を仮定。
 * ======================================================================== */

extern int nsc_info_len; /* 情報ビット長 K   */
extern int nsc_code_len; /* 符号ビット長 N   */

/* ------------------------------------------------------------------------
 * Soft-decision branch metric (1 シンボル分, 2 出力ビット)
 * ------------------------------------------------------------------------
 *  LLR 配列 LLR[] は、BPSK シンボルに対する対数尤度比を想定:
 *      LLR[k] ≈ log( p(y_k | x_k = +1) / p(y_k | x_k = -1) )
 *
 *  出力ビット out_bit0, out_bit1 (0/1) を BPSK (+1/-1) にマップして、
 *  対応する LLR との「負の相関」をブランチメトリックとして用いる。
 *
 *  - シンボル i における 2 つの符号ビットの LLR:
 *        v = LLR[2*i]
 *        w = LLR[2*i + 1]
 *  - ビット 0 → +1, 1 → -1 にマッピング:
 *        s0, s1 ∈ {+1, -1}
 *  - ブランチメトリック:
 *        bm = -(s0 * v + s1 * w)
 *    → v, w の符号と BPSK シンボルの向きが一致しているほど小さくなる。
 * ------------------------------------------------------------------------ */
static inline double branch_metric_soft_symbol(const double *LLR,
                                               int symbol_index, int out_bit0,
                                               int out_bit1) {
  double v = LLR[2 * symbol_index];
  double w = LLR[2 * symbol_index + 1];

  /* 0 → +1, 1 → -1 */
  double s0 = (out_bit0 == 0) ? +1.0 : -1.0;
  double s1 = (out_bit1 == 0) ? +1.0 : -1.0;

  /* 負の相関（値が小さいほど尤度が高いパスとみなす） */
  return -(s0 * v + s1 * w);
}

/* ------------------------------------------------------------------------
 * Hard-decision branch metric (Hamming distance, 1 シンボル分)
 * ------------------------------------------------------------------------
 *  rx_bits[] は 0/1 のハード判定結果を想定。
 *  1 シンボル（2 ビット）の Hamming 距離をブランチメトリックとする。
 * ------------------------------------------------------------------------ */
static inline int branch_metric_hard_symbol(const int *rx_bits,
                                            int symbol_index, int out_bit0,
                                            int out_bit1) {
  int v = rx_bits[2 * symbol_index];
  int w = rx_bits[2 * symbol_index + 1];

  /* 不一致の数 (0,1,2 のいずれか) */
  return (v != out_bit0) + (w != out_bit1);
}

/* ========================================================================
 *  Soft-decision Viterbi Decoder
 *  ----------------------------------------------------------------------
 *  Parameters:
 *      LLR      : 受信 LLR 列 (長さ N = 2*(K + tail_len))
 *      info_hat : 推定情報ビット列 (長さ K)
 *      code_hat : 再符号化系列（任意, NULL の場合は未使用）
 *
 *  説明:
 *      - 標準的な前向き (Forward recursion) + 後ろ向き (Backward trace)
 *        の Viterbi アルゴリズム。
 *      - 終端付きのため、開始状態・終端状態ともに STATE_A を仮定。
 *      - Backward trace 時に、先頭 K ステップ分のみ info_hat に書き込み、
 *        末尾の tail_len ステップ分は破棄する。
 * ======================================================================== */
void nsc_decode_r05_soft(const double *LLR, int *info_hat, int *code_hat) {
  int K = nsc_info_len;
  int N = nsc_code_len;
  int steps = N / 2; /* シンボル数 = K + tail_len */

  /* --------------------------------------------------------------------
   * パスメトリックとバックポインタ
   *   metric_prev[s] : 時刻 i-1 における状態 s のパスメトリック
   *   metric_curr[s] : 時刻 i   における状態 s のパスメトリック
   *
   *   prev_state[i*4 + s] : 時刻 i に状態 s に来たときの「1 つ前の状態」
   *   prev_bit  [i*4 + s] : そのとき入力されたビット (0/1)
   * ------------------------------------------------------------------ */
  double metric_prev[4];
  double metric_curr[4];

  int *prev_state = (int *)malloc(sizeof(int) * steps * 4);
  int *prev_bit = (int *)malloc(sizeof(int) * steps * 4);

  if (!prev_state || !prev_bit) {
    fprintf(stderr, "[NSC Decoder] Memory allocation failed (soft)\n");
    free(prev_state);
    free(prev_bit);
    return;
  }

  /* --------------------------------------------------------------------
   * 初期メトリック（時刻 -1 に相当）
   *   - 終端付きなので、開始状態は必ず STATE_A
   *   - 他状態には非常に大きな値（擬似的な ∞）を与える
   * ------------------------------------------------------------------ */
  for (int s = 0; s < 4; s++) {
    metric_prev[s] = 1e30;
  }
  metric_prev[STATE_A] = 0.0;

  /* ====================================================================
   * Forward recursion
   *   各時刻 i について:
   *     - 各到達可能状態 ps (previous state) と入力ビット b ∈ {0,1}
   *       に対してブランチメトリックを計算し、
   *       遷移先 ns に対する最良パスを更新する。
   * ==================================================================== */
  for (int i = 0; i < steps; i++) {

    /* 各状態のメトリックを一旦「∞」にリセット */
    for (int s = 0; s < 4; s++) {
      metric_curr[s] = 1e30;
    }

    for (int ps = 0; ps < 4; ps++) {
      if (metric_prev[ps] >= 1e29) {
        /* 到達不能な状態はスキップ */
        continue;
      }

      for (int b = 0; b < 2; b++) {

        int ns = nsc_next_state[ps][b];
        int out_bit0 = nsc_output_bits[ps][b][0];
        int out_bit1 = nsc_output_bits[ps][b][1];

        double bm = branch_metric_soft_symbol(LLR, i, out_bit0, out_bit1);
        double cand = metric_prev[ps] + bm;

        /* より良いパスを見つけた場合のみ更新 */
        if (cand < metric_curr[ns]) {
          metric_curr[ns] = cand;
          prev_state[i * 4 + ns] = ps;
          prev_bit[i * 4 + ns] = b;
        }
      }
    }

    /* 次のステップへメトリックをコピー */
    for (int s = 0; s < 4; s++) {
      metric_prev[s] = metric_curr[s];
    }
  }

  /* --------------------------------------------------------------------
   * 終端状態の決定
   *   - tail bits により STATE_A に戻るよう設計しているため、
   *     理論上は state = STATE_A で固定してよい。
   *   - 実装上は、念のため「最小メトリックの状態」を選ぶ。
   * ------------------------------------------------------------------ */
  int state = STATE_A;
  double best_final = metric_prev[state];
  for (int s = 0; s < 4; s++) {
    if (metric_prev[s] < best_final) {
      best_final = metric_prev[s];
      state = s;
    }
  }

  /* ====================================================================
   * Backward trace
   *   - 時刻 i に対応する入力ビット prev_bit[i][state] を取り出し、
   *     そのときの 1 つ前の状態 prev_state[i][state] に戻りながら
   *     逆順に辿っていく。
   *   - i < K のときだけ info_hat[i] にビットを書き込み、
   *     i >= K (tail 区間) は破棄する。
   * ==================================================================== */
  for (int i = steps - 1; i >= 0; i--) {
    int b = prev_bit[i * 4 + state];
    int ps = prev_state[i * 4 + state];

    if (i < K) {
      info_hat[i] = b;
    }

    state = ps;
  }

  /* Optional: 再符号化（デバッグ／自己整合性チェック用） */
  if (code_hat) {
    nsc_encode_r05(info_hat, code_hat);
  }

  free(prev_state);
  free(prev_bit);
}

/* ========================================================================
 *  Hard-decision Viterbi Decoder
 *  ----------------------------------------------------------------------
 *  Parameters:
 *      rx_bits : 受信ビット列 (0/1 のハード判定, 長さ N)
 *      info_hat: 推定情報ビット列 (長さ K)
 *      code_hat: 再符号化系列（任意, NULL の場合は未使用）
 *
 *  説明:
 *      - ブランチメトリックとして Hamming 距離を用いる以外は、
 *        Soft-decision 版と同一の流れ。
 * ======================================================================== */
void nsc_decode_r05_hard(const int *rx_bits, int *info_hat, int *code_hat) {
  int K = nsc_info_len;
  int N = nsc_code_len;
  int steps = N / 2;

  int metric_prev[4];
  int metric_curr[4];

  int *prev_state = (int *)malloc(sizeof(int) * steps * 4);
  int *prev_bit = (int *)malloc(sizeof(int) * steps * 4);

  if (!prev_state || !prev_bit) {
    fprintf(stderr, "[NSC Decoder] Memory allocation failed (hard)\n");
    free(prev_state);
    free(prev_bit);
    return;
  }

  /* --------------------------------------------------------------------
   * 初期メトリック
   *   - STATE_A に 0、それ以外には非常に大きな値を与える。
   * ------------------------------------------------------------------ */
  for (int s = 0; s < 4; s++) {
    metric_prev[s] = 1000000000;
  }
  metric_prev[STATE_A] = 0;

  /* ====================================================================
   * Forward recursion
   *   Hard-decision 版:
   *     - branch_metric_hard_symbol() で Hamming 距離を算出
   * ==================================================================== */
  for (int i = 0; i < steps; i++) {

    for (int s = 0; s < 4; s++) {
      metric_curr[s] = 1000000000;
    }

    for (int ps = 0; ps < 4; ps++) {
      if (metric_prev[ps] >= 100000000) {
        /* 到達不能とみなすしきい値 */
        continue;
      }

      for (int b = 0; b < 2; b++) {

        int ns = nsc_next_state[ps][b];
        int out_bit0 = nsc_output_bits[ps][b][0];
        int out_bit1 = nsc_output_bits[ps][b][1];

        int bm = branch_metric_hard_symbol(rx_bits, i, out_bit0, out_bit1);
        int cand = metric_prev[ps] + bm;

        if (cand < metric_curr[ns]) {
          metric_curr[ns] = cand;
          prev_state[i * 4 + ns] = ps;
          prev_bit[i * 4 + ns] = b;
        }
      }
    }

    for (int s = 0; s < 4; s++) {
      metric_prev[s] = metric_curr[s];
    }
  }

  /* --------------------------------------------------------------------
   * 終端状態（STATE_A を優先）
   *   - 終端付きとはいえ、実測メトリックに基づいて
   *     最小メトリックの状態を選ぶ実装とする。
   * ------------------------------------------------------------------ */
  int state = STATE_A;
  int best_final = metric_prev[state];
  for (int s = 0; s < 4; s++) {
    if (metric_prev[s] < best_final) {
      best_final = metric_prev[s];
      state = s;
    }
  }

  /* ====================================================================
   * Backward trace
   *   Soft-decision 版と同様に、先頭 K ステップのみを info_hat に採用。
   * ==================================================================== */
  for (int i = steps - 1; i >= 0; i--) {
    int b = prev_bit[i * 4 + state];
    int ps = prev_state[i * 4 + state];

    if (i < K) {
      info_hat[i] = b;
    }

    state = ps;
  }

  /* Optional: 再符号化 */
  if (code_hat) {
    nsc_encode_r05(info_hat, code_hat);
  }

  free(prev_state);
  free(prev_bit);
}
