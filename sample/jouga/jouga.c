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

void calibration_func(void);
void jouga_light(void);
void jouga_color(void);
void jouga_dual(void);
void algorithm_light(void);
void algorithm_color(void);
void algorithm_dual(void);
void jouga_straight(void);

/* 外部変数の定義 */
char name[17];
int lval, cval;
int llow = LOWVAL, lhigh = HIGHVAL;
int clow = LOWVAL, chigh = HIGHVAL;
void (*jouga_algorithm)(void) = algorithm_light;	// デフォルトの設定

NameFunc MainMenu[] = {
  {"Main Menu", NULL},
  {"Calibration", calibration_func},	// センサーのキャリブレーション
  {"Light", jouga_light},		// ライトセンサーを選択
  {"Color", jouga_color},		// カラーセンサーを選択
  {"Dual", jouga_dual},			// 2つを使うアルゴリズムを選択
  {"Start", NULL},			// ライントレースの開始
  {"Straight", jouga_straight},		// ペナルティ計測用
  {"Exit", ecrobot_restart_NXT},	// OSの制御に戻る
//  {"Power Off", ecrobot_shutdown_NXT},	// 電源を切る
};

/* ライトセンサーやカラーセンサーの値を0-1023で読み込む(小さいほど暗い) */
int
get_light_sensor(int Sensor)
{
  if (Sensor == Light) {
    return 1024 - 1 - ecrobot_get_light_sensor(Sensor);
  } else {
    return ecrobot_get_nxtcolorsensor_light(Sensor);
  }
}

/* メニューを表示して選択されるのを待つ */
void
func_menu(NameFunc *tbl, int cnt)
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

/* 白と黒のセンサーの読み取り値を校正 */
void
calibration_func(void)
{
  nxtButton btn;
  int lmin, lmax;
  int cmin, cmax;
  int i;

  display_clear(0);
  motor_set_speed(Lmotor, LOWPOWER / 3 + 10, 1);
  motor_set_speed(Rmotor, LOWPOWER / 3 + 10, 1);
  lmin = lmax = get_light_sensor(Light);
  cmin = cmax = get_light_sensor(Color);

  // しばらくの間データを取得
  for (i = 0; i < 150; i++) {
    dly_tsk(20);
    lval = get_light_sensor(Light);
    cval = get_light_sensor(Color);
    if (lval < lmin) lmin = lval;
    if (lval > lmax) lmax = lval;
    if (cval < cmin) cmin = cval;
    if (cval > cmax) cmax = cval;
    display_goto_xy(0, 1);
    display_string("cur: ");
    display_int(lval, 4);
    display_int(cval, 4);
    display_goto_xy(0, 3);
    display_string("min: ");
    display_int(lmin, 4);
    display_int(cmin, 4);
    display_goto_xy(0, 5);
    display_string("max: ");
    display_int(lmax, 4);
    display_int(cmax, 4);
    display_update();
  }
  // データ取得終了
  motor_set_speed(Lmotor, 0, 0);
  motor_set_speed(Rmotor, 0, 0);
  display_goto_xy(3, 7);
  display_string("Is this OK?");
  display_update();
  do {
    btn = get_btn();
    if (btn == Cbtn) return;
  } while (btn != Obtn);
  llow = lmin;
  lhigh = lmax;
  clow = cmin;
  chigh = cmax;
}

/*
 * アルゴリズム選択関数群
 *	ライントレースに使うアルゴリズムを選択する
 *	メニュー実現用として利用
 */
void
jouga_light(void)
{
	jouga_algorithm = algorithm_light;
}

void
jouga_color(void)
{
	jouga_algorithm = algorithm_color;
}

void
jouga_dual(void)
{
	jouga_algorithm = algorithm_dual;
}


/*
 * アルゴリズム実現関数群
 *	実際に機体を動かす
 *	周期タイマがセマフォを操作することで定期的に起動される
 *	ここを直すことで考えているアルゴリズムを実現できる
 */
void
algorithm_light(void)
{
  for(;;) {
    wai_sem(Stskc);	// セマフォを待つことで定期的な実行を実現
    lval = get_light_sensor(Light);
    if (lval > (lhigh + llow) / 2) {		// 閾値より大きいとき
      motor_set_speed(Rmotor, HIGHPOWER, 1);
      motor_set_speed(Lmotor, LOWPOWER, 1);
    } else {					// 閾値より小さいとき
      motor_set_speed(Lmotor, HIGHPOWER, 1);
      motor_set_speed(Rmotor, LOWPOWER, 1);
    }
  }
}

void
algorithm_color(void)
{
  for(;;) {
    wai_sem(Stskc);	// セマフォを待つことで定期的な実行を実現
    cval = get_light_sensor(Color);
    if (cval > (chigh + clow) / 2) {		// 閾値より大きいとき
      motor_set_speed(Lmotor, HIGHPOWER, 1);
      motor_set_speed(Rmotor, LOWPOWER, 1);
    } else {					// 閾値より小さいとき
      motor_set_speed(Rmotor, HIGHPOWER, 1);
      motor_set_speed(Lmotor, LOWPOWER, 1);
    }
  }
}

void
algorithm_dual(void)
{
  for(;;) {
    wai_sem(Stskc);	// セマフォを待つことで定期的な実行を実現
    lval = get_light_sensor(Light);
    cval = get_light_sensor(Color);

    // ライトセンサーと右モーターの関係
    if (lval > (lhigh + llow) / 2) {		// 閾値より明るいとき
      motor_set_speed(Rmotor, HIGHPOWER, 1);
    } else {					// 閾値より暗いとき
      motor_set_speed(Rmotor, LOWPOWER, 1);
    }

    // カラーセンサーと左モーターの関係
    if (cval > (chigh + clow) / 2) {		// 閾値より明るいとき
      motor_set_speed(Lmotor, HIGHPOWER, 1);
    } else {					// 閾値より暗いとき
      motor_set_speed(Lmotor, LOWPOWER, 1);
    }
  }
}

/*
 *　ペナルティ計測用の関数
 *	直進して1500mm進んで止まるだけ
 */
void jouga_straight(void) {
  enum StraightState {
    BlackStraight,
    WhiteStraight,
    ArrivedFirstCircle, 
    ArrivedSecondCircle,
    StateNum,
  };
  // 現在のロボットの状態
  enum StraightState myState = BlackStraight;

  //  完全一致の黒色白色を感知するのは難しいので許容する範囲
  int range = 50;
  // 左右モーターの白領域に入ってからの回転数
  int RmotorCount = 0;
  int LmotorCount = 0;
  int subMotorCount = RmotorCount - LmotorCount;
  bool isAdjusting = false;

  /*---------------発進動作-----------------------*/
  motor_set_speed(Rmotor, HIGHPOWER, 1);
  motor_set_speed(Lmotor, HIGHPOWER, 1);

  for (;;) {
    // 光と色センサーで値取得
    lval = get_light_sensor(Light);
    cval = get_light_sensor(Color);
    
    switch (StraightState) {
      case BlackStraight:
        // 黒ラインアルゴリズム
        // ライトセンサーと右モーターの関係
        if (lval > (lhigh + llow) / 2) {		// 閾値より明るいとき
          motor_set_speed(Rmotor, HIGHPOWER, 1);
        } else {					// 閾値より暗いとき
          motor_set_speed(Rmotor, LOWPOWER, 1);
        }

        // カラーセンサーと左モーターの関係
        if (cval > (chigh + clow) / 2) {		// 閾値より明るいとき
          motor_set_speed(Lmotor, HIGHPOWER, 1);
        } else {					// 閾値より暗いとき
          motor_set_speed(Lmotor, LOWPOWER, 1);
        }
        // 黒ラインを走行中に白を感知したらステートを白走行に変更
        if (cval > chigh - range && lval > lhigh - range) {
          myState = WhiteStraight;
        }
        break;
    
      case WhiteStraight:
        // 完全に0にはならないかも(一つの指標として)
        if (nxt_motor_get_count(Rmotor) == 0) {
          RmotorCount++;
        }
        if (nxt_motor_get_count(Lmotor) == 0) {
          LmotorCount++;
        }
        subMotorCount = RmotorCount - LmotorCount;

        // 回転角度によって速度修正
        if (!isAdjusting) {
          // Rmotorの回転数の方が多いとき
          if (subMotorCount >= 1) {
            motor_set_speed(Rmotor, 0, 1);
            motor_set_speed(Lmotor, HIGHPOWER, 1);
            isAdjusting = true;
          }
          // Lmotorの回転数の方が多いとき
          else if (subRotation <= -1) {
            motor_set_speed(Rmotor, HIGHPOWER, 1);
            motor_set_speed(Lmotor, 0, 1);
            isAdjusting = true;
          }
          else {
            motor_set_speed(Rmotor, HIGHPOWER, 1);
            motor_set_speed(Lmotor, HIGHPOWER, 1);
            isAdjusting = false;
          }
        }

        // 白領域を走行中に黒を感知したらステートを1つ目の円感知後に変更
        if (cval < clow + range && lval < low + range) {
          myState = ArrivedFirstCircle;
          motor_set_speed(Rmotor, LOWPOWER / 3 + 10, 1);
          motor_set_speed(Lmotor, LOWPOWER / 3 + 10, 1);
        }
        break;

      case ArrivedFirstCircle:
        break;

      case ArrivedSecondCircle:
        break;
    }
  }
}

/*
 * TASK: InitTsk
 *	初期設定を行うタスク
 */
void
InitTsk(VP_INT exinf)
{
  display_clear(0);	// なにはともあれ、画面をクリア
  display_goto_xy(2, 3);
  display_string("Initializing");
  display_update();
  // カラーセンサーを使う場合にはライトセンサーとして使う
  // REDが一番ダイナミックレンジが広いようなので、あえてWHITEでなくREDで
  ecrobot_set_nxtcolorsensor(Color, NXT_LIGHTSENSOR_RED);
  ecrobot_get_bt_device_name(name);	// システム名の取得

  act_tsk(Tmain);
}

/*
 * TASK: MainTsk
 *	周期起動用のタイマを起動して終了
 */
void
MainTsk(VP_INT exinf)
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

  // BGM用のタスクを起動
  act_tsk(Tmusc);

  // 移動用のタスクを起動
  act_tsk(Tmove);

  // 表示用のタスクを定期起動するためのタイマを起動
  sta_cyc(Cdisp);
}

/*
 * TASK: MoveTsk
 *	実際に機体を動かす
 *	周期タイマがセマフォを操作することで定期的に起動される
 *	たぶん、ここを直すことで考えているアルゴリズムを実現できる
 */
void
MoveTsk(VP_INT exinf)
{
  sta_cyc(Cmove);	// 定期的にセマフォを上げるタイマ

  (*jouga_algorithm)();	// 実際の処理
}

/*
 * TASK: MuscTsk
 *	BGMを奏でる
 *	実体はmusic.cにある
 */
void
MuscTsk(VP_INT exinf)
{
  // 延々と大学歌を奏で続ける
  for (;;) {
    play_notes(TIMING_chiba_univ, 8, chiba_univ);
  }
}

/*
 * TASK: DispTsk
 *	通常動作時にシステム内の様子を表示
 *	周期タイマにより定期的に起動される
 */
void
DispTsk(VP_INT exinf)
{
  display_clear(0);

  /* システム名の表示 */
  display_goto_xy(0, 0);
  display_string(name);
  display_string(" status");

  /* センサーの読み取り値の表示 */
  display_goto_xy(3, 3);
  display_int(lval, 4);
  display_string("  ");
  display_int(cval, 4);

  display_update();
}

/*
 * TASK: IdleTsk
 *	Idle時に動作する（優先順位は低い）
 *	Cyclic Timerでなくdly_tskしているのは、
 *	セマフォで待っているときに複数起動しても意味がないため
 */
void
IdleTsk(VP_INT exinf)
{
  for (;;) {
    wai_sem(Snbtn);		// InitTskとNXTボタンを取り合う
    check_NXT_buttons();
    if (ecrobot_is_ENTER_button_pressed()) {
      stp_cyc(Cmove);
      stp_cyc(Cdisp);
      ter_tsk(Tmove);
      ter_tsk(Tmusc);
      ter_tsk(Tmain);
      nxt_motor_set_speed(Rmotor, 0, 0);
      nxt_motor_set_speed(Lmotor, 0, 0);
      act_tsk(Tmain);
    }
    sig_sem(Snbtn);		// NXTボタンの権利を返却
    dly_tsk(10);
  }
}

/*
 * TASK: ColsTsk
 *	Idle時にカラーセンサー用に値を読み込む
 */
void
ColsTsk(VP_INT exinf)
{
  for (;;) {
    ecrobot_process_bg_nxtcolorsensor();
    dly_tsk(2);
  }
}

/*
 * 周期タイマ
 *	タスクを定期的に起動するだけ
 */
void
MoveCyc(VP_INT exinf)
{
  isig_sem(Stskc);	// MoveTskを進めるためにセマフォを操作
}

void
DispCyc(VP_INT exinf)
{
  iact_tsk(Tdisp);	// DispTskを定期的に起動
}

/* OSにより1msごとに呼び出される */
void
jsp_systick_low_priority(void)
{
  if (get_OS_flag()) {
    isig_tim();		// 今回はタイマを使っているのでこの呼び出しが必要
  }
}

/* システムの初期化ルーチン */
void
ecrobot_device_initialize(void)
{
  nxt_motor_set_speed(Rmotor, 0, 0);
  nxt_motor_set_speed(Lmotor, 0, 0);
  ecrobot_init_nxtcolorsensor(Color, NXT_COLORSENSOR);
  ecrobot_set_light_sensor_active(Light);
}

/* システム停止時に呼ばれるルーチン */
void
ecrobot_device_terminate(void)
{
  nxt_motor_set_speed(Rmotor, 0, 1);
  nxt_motor_set_speed(Lmotor, 0, 1);
  ecrobot_term_nxtcolorsensor(Color);
  ecrobot_set_light_sensor_inactive(Light);
}
