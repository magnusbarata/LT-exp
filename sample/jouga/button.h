/*
 * button.h
 *	ボタン操作用ヘッダファイル
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "mytypes.h"		// U8の定義が必要

// ボタンを表す記号定数
// 複数のボタンは論理和を取ってあらわす
typedef enum {
  Obtn = 0x01,
  Lbtn = 0x02,
  Rbtn = 0x04,
  Cbtn = 0x08
} nxtButtons;

typedef U8 nxtButton;		// プログラム中で用いるボタンをあらわす型

void click_btn(nxtButton);	// クリック音を鳴らす
void wait_for_release(void);	// 押されているボタンが離されるまで待つ
nxtButton get_btn(void);	// いずれかのボタンが押されるのを待つ

#endif
