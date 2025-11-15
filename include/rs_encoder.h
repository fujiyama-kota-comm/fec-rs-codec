#ifndef RS_ENCODER_H
#define RS_ENCODER_H

/* RS エンコーダ
 * 入力: 情報ビット列 (長さ K*m)
 * 出力: 符号語ビット列 (長さ N*m)
 */
void rs_encode(const int *inf_bits, int *code_bits);

#endif /* RS_ENCODER_H */
