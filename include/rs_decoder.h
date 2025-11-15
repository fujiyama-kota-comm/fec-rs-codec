#ifndef RS_DECODER_H
#define RS_DECODER_H

/* RS デコーダ
 * 入力: 受信ビット列 (長さ N*m)
 * 出力: 復号後のコード語ビット列 code_bits (長さ N*m)
 *       復号後の情報ビット列 info_bits (長さ K*m)
 */
void rs_decode(const int *recv_bits, int *code_bits, int *info_bits);

#endif /* RS_DECODER_H */
