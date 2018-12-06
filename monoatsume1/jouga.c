/*
 *	TOPPERS/JSPを用いたライントレーサーのサンプルコード
 */

/*
  TODO:
    MotrTsk(), MoveTsk(), movelength(), calibration(), collect_all()
  EXP:
    Display, Sound, Motor, Arm, Collect All
*/
#include "display.h"	// 変更したバージョンを使うために先頭でinclude

#include "kernel_id.h"
#include "ecrobot_interface.h"
#include "ecrobot_base.h"

#include "jouga_cfg.h"
#include "jouga.h"
#include "music.h"
#include "graphics.h"

#include <t_services.h>
#include "ecrobot_interface.h"
#include "mytypes.h"		// U8の定義が必要

#define ARRAYSIZE(A)	(sizeof((A)) / sizeof((A)[0]))

/* 型や関数の宣言 */
typedef void (*MFunc)(void);
typedef struct _NameFunc {
  char *name;
  MFunc func;
} NameFunc;

typedef enum {
	BLK = 1 << 0,  // 黒
	BLU = 1 << 1,  // 青
  GRN = 1 << 2,  // 緑
	CYA = 1 << 3,  // シアン
  RED = 1 << 4,  // 赤
  MAG = 1 << 5,  // マゼンタ
  YEL = 1 << 6,  // 黄
  WHT = 1 << 7,  // 白
  RTP = 1 << 8,  // 右押す
  RTR = 1 << 9,  // 右離す
  LTP = 1 << 10, // 左押す
  LTR = 1 << 11, // 左離す
  DIS = 1 << 12, // 移動距離
  POS = 1 << 13, // アーム位置
  // 以下ライトセンサー?
} EBits;

typedef enum {
  Obtn = 1 << 0,
  Lbtn = 1 << 1,
  Rbtn = 1 << 2,
  Cbtn = 1 << 3,
} Nbtns;

U8 ecrobot_get_button_state(void);
void ecrobot_poll_nxtstate(void);

void calibrate(void);
void collect_all(void);
void calibration(void);
void alg_collect_all(void);

/* 外部変数の定義 */
char name[17];
void (*algorithm)(void) = alg_collect_all;
int Adeg = 0; // アームモーターの目的角度
int Apow = 0; // アームの上下パワー

/*----------- 雑多な関数群 -----------*/
void wait_for_release(void)
{
  while (ecrobot_get_button_state()) {
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  }
}

U8 bin(const int val, const int div, const int n)
{
  if(val > div) return 1 << n;
  return 0 << n;
}

U8 get_btn(void)
{
  U8 btn, t;

  do {
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  } while (!(btn = ecrobot_get_button_state()));
  while ((t = ecrobot_get_button_state())) {
    btn |= t;
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  }
  return btn;
}

/*----------- メニュー -----------*/
NameFunc MainMenu[] = {
  {"Main Menu", NULL},
  {"Start", NULL},
  {"Calibration", calibrate},
  {"Collect All", collect_all},
  {"Exit", ecrobot_restart_NXT},	// OSの制御に戻る
//  {"Power Off", ecrobot_shutdown_NXT},	// 電源を切る
};

/* メニューを表示して選択されるのを待つ */
void func_menu(NameFunc *tbl, int cnt)
{
  int i;
  static int menu = 1;
  U8 btn;

  for (;;) {
    display_clear(0);
    display_goto_xy(0, 0);
    display_string_inverted(tbl[0].name);	// 変更済みのdisplay.cのみ
    for (i = 1; i < cnt; i++) {
      if (i == menu) {
        nxt_display_dish(nxt_display_set, 6, i * 8 + 3, 3);
      }
      display_goto_xy(2, i);
      display_string(tbl[i].name);
    }
    display_update();
    btn = get_btn();
    switch (btn) {
    case Obtn:	// オレンジボタン == 選択
      if (tbl[menu].func == NULL) break;
      tbl[menu].func();		// メニューの項目を実行
      continue;
    case Cbtn:	// グレーボタン == キャンセル
      continue;
    case Rbtn:	// 右ボタン == 次へ
      menu++;
      if (menu >= cnt) menu = 1;
      continue;
    case Lbtn:	// 左ボタン == 前へ
      --menu;
      if (menu <= 0) menu = cnt - 1;
      continue;
    default:	// 複数が押されている場合
      continue;
    }
    break;
  }
}

void calibrate(void) {algorithm = calibration;}
void calibration(void)
{
  iact_tsk(Tmotr);
}

/*----------- アルゴリズム群 -----------*/
void collect_all(void) {algorithm = alg_collect_all;}
void alg_collect_all(void)
{
  // TODO
  //if () set_flg(Fsens, DIS);
}

void collect_red_ball(void) {algorithm = alg_collect_all;}
void alg_collect_red_ball(void)
{

}

/*----------- タスク群 -----------*/
void SensTsk(VP_INT exinf)
{
  static int COL_THRES[] = {400, 350, 320};
  S16 col[3];
  U8 CBits = 0;

	for (;;) {
		dly_tsk(5);

    // カラーセンサー
    ecrobot_get_nxtcolorsensor_rgb(Color, col);
    CBits = bin(col[0], COL_THRES[0], 2) |
            bin(col[1], COL_THRES[1], 1) |
            bin(col[2], COL_THRES[2], 0);
    // フラッグをクリアしてからセットする
    clr_flg(Fsens, ~(BLK | BLU | GRN | CYA |
                     RED | MAG | YEL | WHT));
    switch(CBits){
      case 0: set_flg(Fsens, BLK); break;
      case 1: set_flg(Fsens, BLU); break;
      case 2: set_flg(Fsens, GRN); break;
      case 3: set_flg(Fsens, CYA); break;
      case 4: set_flg(Fsens, RED); break;
      case 5: set_flg(Fsens, MAG); break;
      case 6: set_flg(Fsens, YEL); break;
      case 7: set_flg(Fsens, WHT); break;
    }

    // タッチセンサー
		if (ecrobot_get_touch_sensor(Rtouch)) {
			set_flg(Fsens, RTP);
      clr_flg(Fsens, RTR);
		} else {
      set_flg(Fsens, RTR);
      clr_flg(Fsens, RTP);
    }

		if (ecrobot_get_touch_sensor(Ltouch)) {
			set_flg(Fsens, LTP);
      clr_flg(Fsens, LTR);
		} else {
      set_flg(Fsens, LTR);
      clr_flg(Fsens, LTP);
    }
	}
}

void NbtnTsk(VP_INT exinf)
{
  U8 btn;

  for (;;){
    btn = ecrobot_get_button_state();
    dly_tsk(2);
    switch(btn){
      case Obtn: set_flg(Fnbtn, Obtn); break;
      case Lbtn: set_flg(Fnbtn, Lbtn); break;
      case Rbtn: set_flg(Fnbtn, Rbtn);; break;
      case Cbtn: set_flg(Fnbtn, Cbtn); break;
      default: ecrobot_poll_nxtstate(); break;
    }
  }
}

void QuitTsk(VP_INT exinf)
{
  FLGPTN BtnSens;

  for (;;) {
    wai_sem(Snbtn);
    wai_flg(Fnbtn, Cbtn, TWF_ORW, &BtnSens);
    if(Cbtn){
      nxt_motor_set_speed(Rmotor, 0, 0);
      nxt_motor_set_speed(Lmotor, 0, 0);
      nxt_motor_set_speed(Amotor, 0, 1);
      //stp_cyc(Cmove);
      stp_cyc(Cdisp);
      //ter_tsk(Tmove);
      ter_tsk(Ttimr);
      ter_tsk(Tmusc);
      ter_tsk(Tmain);
      act_tsk(Tinit);
    }
    /*check_NXT_buttons();
    if (ecrobot_is_ENTER_button_pressed()) {
      nxt_motor_set_speed(Rmotor, 0, 0);
      nxt_motor_set_speed(Lmotor, 0, 0);
      nxt_motor_set_speed(Amotor, 0, 1);
      //stp_cyc(Cmove);
      stp_cyc(Cdisp);
      //ter_tsk(Tmove);
      ter_tsk(Ttimr);
      ter_tsk(Tmusc);
      ter_tsk(Tmain);
      act_tsk(Tinit);
    }*/
    sig_sem(Snbtn);
    dly_tsk(10);
  }
}

void InitTsk(VP_INT exinf)
{
  display_clear(0);	// なにはともあれ、画面をクリア
  display_goto_xy(2, 3);
  display_string("Initializing");
  display_update();
  ecrobot_get_bt_device_name(name);	// システム名の取得
  act_tsk(Tsens);
  act_tsk(Tnbtn);
  act_tsk(Tdisp);
  act_tsk(Tmove);

  // ボタンが押されていない状態になるまで待つ
  wait_for_release();
  wai_sem(Snbtn);	// ボタンに関する権利を取得
  func_menu(MainMenu, ARRAYSIZE(MainMenu)); // メインメニューの表示
  sig_sem(Snbtn);	// ボタンに関する権利を開放
}

void MainTsk(VP_INT exinf)
{
  // 画面をきれいにする
  display_clear(0);
  display_goto_xy(0, 0);
  display_update();

  act_tsk(Tquit);
  act_tsk(Ttimr);
  act_tsk(Tmusc);

  //(*algorithm)();
  sta_cyc(Cdisp); // Before (*algorithm)()?
}

void MoveTsk(VP_INT exinf)
{
  sta_cyc(Cmove);	// 定期的にセマフォを上げるタイマ

  //(*algorithm)();	// 実際の処理
}

void MotrTsk(VP_INT exinf)
{
  FLGPTN ArmSens;

  nxt_motor_set_count(Amotor, 0);
  clr_flg(Fsens, ~POS);
  // 無限ループ?
  do{
    wai_flg(Fsens, POS, TWF_ORW, &ArmSens);
    motor_set_speed(Amotor, Apow, 1);
    if (nxt_motor_get_count(Amotor) == Adeg){
      // Can motor degree be minus?
      motor_set_speed(Amotor, 0, 1);
      set_flg(Fsens, POS);
    }
  } while(!POS);
}

void TimrTsk(VP_INT exinf)
{
  static unsigned int s = 0;
  for(;;){
    // 10秒ごとに音鳴らす
    dly_tsk(1000);
    s++;
    if(!(s%10)) ecrobot_sound_tone(220, 100, 60);
  }
}

void DispTsk(VP_INT exinf)
{
  FLGPTN sens;

  display_clear(0);

  /* Header */
  display_goto_xy(0, 0);
  display_string(name);

  /* Footer */
  // カラー表示
  display_goto_xy(0, 7);
  wai_flg(Fsens, BLK | BLU | GRN | CYA | RED |
                 MAG | YEL | WHT | RTP | LTP |
                 RTR | LTR | POS | DIS, TWF_ORW, &sens);

  switch(sens){
    case BLK: display_string("K"); break;
    case BLU: display_string("B"); break;
    case GRN: display_string("G"); break;
    case CYA: display_string("C"); break;
    case RED: display_string("R"); break;
    case MAG: display_string("M"); break;
    case YEL: display_string("Y"); break;
    case WHT: display_string("W"); break;
    //default: display_string(" ");
  }

  // タッチ表示
  switch(sens){
    case LTP:
      display_goto_xy(1, 7); display_string("[");
      break;
    case RTP:
      display_goto_xy(2, 7); display_string("]");
      break;
    case LTP | RTP:
      display_goto_xy(1, 7); display_string("[]");
      break;
    default:
      display_goto_xy(1, 7); display_string("--");
      break;
  }

  // モーターとアーム
  display_goto_xy(3, 7); display_string("--");
  if(sens){
    display_goto_xy(3, 7);
    display_string("D");
  }
  if(sens){
    display_goto_xy(4, 7);
    display_string("P");
  }

  display_update();
}

void MuscTsk(VP_INT exinf)
{
  FLGPTN ColSens;
  // 延々と大学歌を奏で続ける
  for (;;) {
    wai_flg(Fsens,
      BLK | BLU | GRN | CYA |
      RED | MAG | YEL | WHT, TWF_ORW, &ColSens);

    // 色ごとに音が変わる(C4からC5まで)
    switch(ColSens){
      case BLK: ecrobot_sound_tone(262, 100, 60); break;
      case BLU: ecrobot_sound_tone(294, 100, 60); break;
      case GRN: ecrobot_sound_tone(330, 100, 60); break;
      case CYA: ecrobot_sound_tone(349, 100, 60); break;
      case RED: ecrobot_sound_tone(392, 100, 60); break;
      case MAG: ecrobot_sound_tone(440, 100, 60); break;
      case YEL: ecrobot_sound_tone(494, 100, 60); break;
      case WHT: //ecrobot_sound_tone(523, 100, 60);
        break;
    }

    //play_notes(TIMING_chiba_univ, 8, chiba_univ);
    // TODO: 状態ごとに音変わる ()
  }
}

void ColsTsk(VP_INT exinf)
{
  for (;;) {
    ecrobot_process_bg_nxtcolorsensor();
    dly_tsk(2);
  }
}

/*----------- 周期 タイマー群 -----------*/
void MoveCyc(VP_INT exinf)
{
  isig_sem(Stskc);	// MoveTskを進めるためにセマフォを操作
}

void DispCyc(VP_INT exinf)
{
  iact_tsk(Tdisp);	// DispTskを定期的に起動
}

/* OSにより1msごとに呼び出される */
void jsp_systick_low_priority(void)
{
  if (get_OS_flag()) {
    isig_tim();		// 今回はタイマを使っているのでこの呼び出しが必要
  }
}

/*----------- システムフック関数群 -----------*/
void ecrobot_device_initialize(void)
{
  nxt_motor_set_speed(Rmotor, 0, 0);
  nxt_motor_set_speed(Lmotor, 0, 0);
  nxt_motor_set_speed(Amotor, 0, 0);
  ecrobot_init_nxtcolorsensor(Color, NXT_COLORSENSOR);
  //ecrobot_set_light_sensor_active(Light);
  //ecrobot_init_sonar_sensor(Sonar);
}

void ecrobot_device_terminate(void)
{
  nxt_motor_set_speed(Rmotor, 0, 1);
  nxt_motor_set_speed(Lmotor, 0, 1);
  nxt_motor_set_speed(Amotor, 0, 1);
  ecrobot_term_nxtcolorsensor(Color);
  //ecrobot_set_light_sensor_inactive(Light);
  //ecrobot_term_sonar_sensor(Sonar);
}
