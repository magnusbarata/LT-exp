/*
 *	NXT本体ボタンを操作するための関数群
 */ 

#include <t_services.h>
#include "ecrobot_interface.h"
#include "button.h"

// これらの宣言はOSの内部状態を見ているため本来は好ましくない
// 全てのボタンをアプリケーションで利用できるように参照している
U8 ecrobot_get_button_state(void);
void ecrobot_poll_nxtstate(void);

/* ボタンの状態に対応したクリック音の周波数 */
int tone[] = {
  0,
   392,
    349,
   392,
     440,
   392,
    349,
   392,
      262,
   392,
    349,
   392,
     440,
   392, 
    349,
  392
};

/* クリック音を鳴らす */
void
click_btn(nxtButton btn)
{
  if (btn) {
    ecrobot_sound_tone(tone[btn], 30, 40);
  }
}

/* 押されているボタンが離されるまで待つ */
void
wait_for_release(void)
{
  while (ecrobot_get_button_state()) {
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  }
}

/* いずれかのボタンが押されるのを待つ */
nxtButton
get_btn(void)
{
  nxtButton btn, t;

  do {
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  } while (!(btn = ecrobot_get_button_state()));
  click_btn(btn);
  while ((t = ecrobot_get_button_state())) {
    btn |= t;
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  }
  return btn;
}
