/*
 *	TOPPERS/JSPを用いたライントレーサーのサンプルコード
 */

#include "display.h"	// 変更したバージョンを使うために先頭でinclude

#include "kernel_id.h"
#include "ecrobot_interface.h"
#include "ecrobot_base.h"

#include "jouga_cfg.h"
#include "jouga.h"
#include "music.h"
#include "button.h"
#include "graphics.h"

#define ARRAYSIZE(A)	(sizeof((A)) / sizeof((A)[0]))

/* 型や関数の宣言 */
typedef void (*MFunc)(void);
typedef struct _NameFunc {
  char *name;
  MFunc func;
} NameFunc;

typedef enum
{
	BCK = 1 << 0,
	BLU = 1 << 1,
  GRN = 1 << 2,
	CYA = 1 << 3,
  RED = 1 << 4,
  MAG = 1 << 5,
  YEL = 1 << 6,
  WHT = 1 << 7,
  RTS = 1 << 8,
  LTS = 1 << 9,
} EBits;

void calibration_func(void);
void jouga_collect(void);
void algorithm_collect(void);

/* 外部変数の定義 */
char name[17] = "ver1.0";
void (*jouga_algorithm)(void) = algorithm_collect;


/*----------- Menus -----------*/
NameFunc MainMenu[] = {
  {"Main Menu", NULL},
  {"Start", NULL},			// ライントレースの開始
  {"Collect", jouga_collect},
  {"Exit", ecrobot_restart_NXT},	// OSの制御に戻る
//  {"Power Off", ecrobot_shutdown_NXT},	// 電源を切る
};

/* メニューを表示して選択されるのを待つ */
void func_menu(NameFunc *tbl, int cnt)
{
  int i;
  static int menu = 1;
  nxtButton btn;

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
      if (tbl[menu].func == NULL)
	break;
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

void calibration_func(void)
{
  iact_tsk(Tmotr);
  iact_tsk(Tmain);
}

/*----------- Algorithms -----------*/
void jouga_collect(void){
  jouga_algorithm = algorithm_collect;
}

void algorithm_collect(void){

}

/*----------- Tasks -----------*/
void SensTsk(VP_INT exinf)
{
	/*for (;;) {
		dly_tsk(10);
		if (ecrobot_get_touch_sensor(Rtouch)) {
			set_flg(Fsens, eRtouch);
		}
		if (ecrobot_get_touch_sensor(Ltouch)) {
			set_flg(Fsens, eLtouch);
		}
	}*/
}

void NbtnTsk(VP_INT exinf)
{
}

void QuitTsk(VP_INT exinf)
{
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

  calibration_func();
}

void MainTsk(VP_INT exinf)
{
  // ここにくるのにボタンを押しているので、
  // ボタンが押されていない状態になるまで待つ
  wait_for_release();
  wai_sem(Snbtn);	// ボタンに関する権利を取得
  // メインメニューの表示
  func_menu(MainMenu, ARRAYSIZE(MainMenu));
  sig_sem(Snbtn);	// ボタンに関する権利を開放

  // 画面をきれいにする
  display_clear(0);
  display_goto_xy(0, 0);
  display_update();

  act_tsk(Tquit);
  act_tsk(Ttimr);
  act_tsk(Tmusc);

}

void MoveTsk(VP_INT exinf)
{
  sta_cyc(Cmove);	// 定期的にセマフォを上げるタイマ

  //(*jouga_algorithm)();	// 実際の処理
}

void MotrTsk(VP_INT exinf)
{
}

void TimrTsk(VP_INT exinf)
{
}

void DispTsk(VP_INT exinf)
{
  display_clear(0);

  /* Header */
  display_goto_xy(0, 0);
  display_string(name);
  display_string(" abc");

  /* センサーの読み取り値の表示 */
  /*display_goto_xy(3, 3);
  display_int(lval, 4);
  display_string("  ");
  display_int(cval, 4);*/

  /* Footer */
  display_goto_xy(0, 0);
  display_string(name);
  display_string(" abc");

  display_update();
}

void MuscTsk(VP_INT exinf)
{
  // 延々と大学歌を奏で続ける
  for (;;) {
    play_notes(TIMING_chiba_univ, 8, chiba_univ);
  }
}

void ColsTsk(VP_INT exinf)
{
  for (;;) {
    ecrobot_process_bg_nxtcolorsensor();
    dly_tsk(2);
  }
}

/*----------- Cyclic Timer -----------*/
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

/* システムの初期化ルーチン */
void ecrobot_device_initialize(void)
{
  nxt_motor_set_speed(Rmotor, 0, 0);
  nxt_motor_set_speed(Lmotor, 0, 0);
  nxt_motor_set_speed(Amotor, 0, 0);
  ecrobot_init_nxtcolorsensor(Color, NXT_COLORSENSOR);
  //ecrobot_set_light_sensor_active(Light);
  //ecrobot_init_sonar_sensor(Sonar);
}

/* システム停止時に呼ばれるルーチン */
void ecrobot_device_terminate(void)
{
  nxt_motor_set_speed(Rmotor, 0, 1);
  nxt_motor_set_speed(Lmotor, 0, 1);
  nxt_motor_set_speed(Amotor, 0, 1);
  ecrobot_term_nxtcolorsensor(Color);
  //ecrobot_set_light_sensor_inactive(Light);
  //ecrobot_term_sonar_sensor(Sonar);
}
