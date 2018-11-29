/*
 *	TOPPERS/JSPを用いたライントレーサーのサンプルコード
 */

#include "display.h" // 変更したバージョンを使うために先頭でinclude

#include "kernel_id.h"
#include "ecrobot_interface.h"
#include "ecrobot_base.h"

#include "jouga_cfg.h"
#include "jouga.h"
#include "music.h"
#include "button.h"
#include "graphics.h"

#define ARRAYSIZE(A) (sizeof((A)) / sizeof((A)[0]))

/* 型や関数の宣言 */
typedef void (*MFunc)(void);
typedef struct _NameFunc
{
  char *name;
  MFunc func;
} NameFunc;

typedef enum
{
  BLACK = 0,
  BLUE,
  GREEN,
  CYAN,
  RED,
  MAGENTA,
  YELLOW,
  WHITE,
  COLORNUM,
} CalcColor;

void calibration_func(void);
void algorithm_collect(void);

/* Displaying Sensor Output */
void dispColor_func(void);
void dispTouch_func(void);
void dispSonar_func(void);

/* 外部変数の定義 */
char name[17];
int lval, cval;
int llow = LOWVAL, lhigh = HIGHVAL;
int clow = LOWVAL, chigh = HIGHVAL;
void (*jouga_algorithm)(void) = jouga_collect; // デフォルトの設定

NameFunc MainMenu[] = {
    {"Main Menu", NULL},
    {"Calibration", calibration_func}, // センサーのキャリブレーション
    {"Display Color", dispColor_func},
    {"Display Touch", dispTouch_func},
    {"Display Sonar", dispSonar_func},
    {"Start", NULL},               // ライントレースの開始
    {"Exit", ecrobot_restart_NXT}, // OSの制御に戻る
                                   //  {"Power Off", ecrobot_shutdown_NXT},	// 電源を切る
};

/* ライトセンサーやカラーセンサーの値を0-1023で読み込む(小さいほど暗い) */
int get_light_sensor(int Sensor)
{
  if (Sensor == Light)
  {
    return 1024 - 1 - ecrobot_get_light_sensor(Sensor);
  }
  else
  {
    return ecrobot_get_nxtcolorsensor_light(Sensor);
  }
}

/* メニューを表示して選択されるのを待つ */
void func_menu(NameFunc *tbl, int cnt)
{
  int i;
  static int menu = 1;
  nxtButton btn;

  for (;;)
  {
    display_clear(0);
    display_goto_xy(0, 0);
    display_string_inverted(tbl[0].name); // 変更済みのdisplay.cのみ
    for (i = 1; i < cnt; i++)
    {
      if (i == menu)
      {
        nxt_display_dish(nxt_display_set, 6, i * 8 + 3, 3);
      }
      display_goto_xy(2, i);
      display_string(tbl[i].name);
    }
    display_update();
    btn = get_btn();
    switch (btn)
    {
    case Obtn: // オレンジボタン == 選択
      if (tbl[menu].func == NULL)
        break;
      tbl[menu].func(); // メニューの項目を実行
      continue;
    case Cbtn: // グレーボタン == キャンセル
      continue;
    case Rbtn: // 右ボタン == 次へ
      menu++;
      if (menu >= cnt)
        menu = 1;
      continue;
    case Lbtn: // 左ボタン == 前へ
      --menu;
      if (menu <= 0)
        menu = cnt - 1;
      continue;
    default: // 複数が押されている場合
      continue;
    }
    break;
  }
}

/* 白と黒のセンサーの読み取り値を校正 */
void calibration_func(void)
{
  nxtButton btn;
  int lmin, lmax;
  int cmin, cmax;
  int i;

  display_clear(0);
  motor_set_speed(Lmotor, LOWPOWER / 3 + 10, 1);
  motor_set_speed(Rmotor, LOWPOWER / 3 + 10, 1);
  //lmin = lmax = get_light_sensor(Light);
  cmin = cmax = get_light_sensor(Color);

  // しばらくの間データを取得
  for (i = 0; i < 150; i++)
  {
    dly_tsk(20);
    //lval = get_light_sensor(Light);
    cval = get_light_sensor(Color);
    if (lval < lmin)
      lmin = lval;
    if (lval > lmax)
      lmax = lval;
    if (cval < cmin)
      cmin = cval;
    if (cval > cmax)
      cmax = cval;
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
  do
  {
    btn = get_btn();
    if (btn == Cbtn)
      return;
  } while (btn != Obtn);
  llow = lmin;
  lhigh = lmax;
  clow = cmin;
  chigh = cmax;
}

/* Displaying Sensor Output */
U8 bin(const int val, const int div, const int n)
{
  if (val > div)
    return 1 << n;
  return 0 << n;
}
U8 bits = 0;
void dispColor_func(void)
{
  S16 col[3];

  ecrobot_set_nxtcolorsensor(Color, NXT_COLORSENSOR);
  display_clear(0);
  for (;;)
  {
    // Read Color value
    dly_tsk(100);
    ecrobot_get_nxtcolorsensor_rgb(Color, col);
    bits = bin(col[0], 400, 2) |
           bin(col[1], 350, 1) |
           bin(col[2], 320, 0);

    // Display Color
    display_goto_xy(2, 2);
    display_int(col[0], 4);
    display_int(col[1], 4);
    display_int(col[2], 4);
    display_goto_xy(3, 7);
    switch (bits)
    {
    case BLACK:
      display_string("BLACK");
      break;
    case BLUE:
      display_string("BLUE");
      break;
    case GREEN:
      display_string("GREEN");
      break;
    case CYAN:
      display_string("CYAN");
      break;
    case RED:
      display_string("RED");
      break;
    case MAGENTA:
      display_string("MAGENTA");
      break;
    case YELLOW:
      display_string("YELLOW");
      break;
    case WHITE:
      display_string("WHITE");
      break;
    }
    display_update();
  }
}

U8 flag1;
U8 flag2;
void dispTouch_func(void)
{
  // Touch sensor display
  nxtButton btn;
  display_clear(0);

  for (;;)
  {
    flag1 = ecrobot_get_touch_sensor(Rtouch); //if 1 = on, 0 = off
    flag2 = ecrobot_get_touch_sensor(Ltouch); //if 1 = on, 0 = off
    switch (btn)
    { // btn = get_btn()?
    case Cbtn:
      break;
    default:
      continue;
    }
    break;
  }
}

void dispSonar_func(void)
{
  for (;;)
  {
    display_goto_xy(0, 2);
    display_int(ecrobot_get_sonar_sensor(Sonar), 4);
    display_update();
  }
}

/*
 * アルゴリズム実現関数群
 *	実際に機体を動かす
 *	周期タイマがセマフォを操作することで定期的に起動される
 *	ここを直すことで考えているアルゴリズムを実現できる
 */

void jouga_collect(void)
{
  jouga_algorithm = algorithm_collect;
}

void algorithm_collect(void)
{
  S16 col[3];
  for (;;)
  {
    wai_sem(Stskc); // セマフォを待つことで定期的な実行を実現

    flag1 = ecrobot_get_touch_sensor(Rtouch); //if 1 = on, 0 = off
    flag2 = ecrobot_get_touch_sensor(Ltouch); //if 1 = on, 0 = off
    ecrobot_set_nxtcolorsensor(Color, NXT_COLORSENSOR);
    ecrobot_get_nxtcolorsensor_rgb(Color, col);
    bits = bin(col[0], 400, 2) |
           bin(col[1], 350, 1) |
           bin(col[2], 320, 0);
  }
}

/*
 * TASK: InitTsk
 *	初期設定を行うタスク
 */
void InitTsk(VP_INT exinf)
{
  display_clear(0); // なにはともあれ、画面をクリア
  display_goto_xy(2, 3);
  display_string("Initializing");
  display_update();
  // カラーセンサーを使う場合にはライトセンサーとして使う
  // REDが一番ダイナミックレンジが広いようなので、あえてWHITEでなくREDで
  ecrobot_set_nxtcolorsensor(Color, NXT_LIGHTSENSOR_RED);
  ecrobot_get_bt_device_name(name); // システム名の取得

  act_tsk(Tmain);
}

/*
 * TASK: MainTsk
 *	周期起動用のタイマを起動して終了
 */
void MainTsk(VP_INT exinf)
{
  // ここにくるのにボタンを押しているので、
  // ボタンが押されていない状態になるまで待つ
  wait_for_release();
  wai_sem(Snbtn); // ボタンに関する権利を取得
  // メインメニューの表示
  func_menu(MainMenu, ARRAYSIZE(MainMenu));
  sig_sem(Snbtn); // ボタンに関する権利を開放

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
void MoveTsk(VP_INT exinf)
{
  sta_cyc(Cmove); // 定期的にセマフォを上げるタイマ

  //(*jouga_algorithm)();	// 実際の処理
}

/*
 * TASK: DispTsk
 *	通常動作時にシステム内の様子を表示
 *	周期タイマにより定期的に起動される
 */
void DispTsk(VP_INT exinf)
{
  display_clear(0);

  /* システム名の表示 */
  display_goto_xy(0, 0);
  display_string(name);
  display_string("state"); // 現在のマシンの状態を表示します.
  display_int(ecrobot_get_systick_ms, 3);
  /* センサーの読み取り値の表示 */
  display_goto_xy(0, 6);
  display_string("Rt:");
  display_int(flag1, 2);
  display_string(" Lt:");
  display_int(flag2, 2);
  display_string("C:");
  display_int(bits, 2);

  display_update();
}
/*
 * TASK: IdleTsk
 *	Idle時に動作する（優先順位は低い）
 *	Cyclic Timerでなくdly_tskしているのは、
 *	セマフォで待っているときに複数起動しても意味がないため
 */
void IdleTsk(VP_INT exinf)
{
  for (;;)
  {
    wai_sem(Snbtn); // InitTskとNXTボタンを取り合う
    check_NXT_buttons();
    if (ecrobot_is_ENTER_button_pressed())
    {
      stp_cyc(Cmove);
      stp_cyc(Cdisp);
      ter_tsk(Tmove);
      ter_tsk(Tmusc);
      ter_tsk(Tmain);
      nxt_motor_set_speed(Rmotor, 0, 0);
      nxt_motor_set_speed(Lmotor, 0, 0);
      act_tsk(Tmain);
    }
    sig_sem(Snbtn); // NXTボタンの権利を返却
    dly_tsk(10);
  }
}

void MuskTsk(VP_INT exinf)
{
}
/*
 * TASK: ColsTsk
 *	Idle時にカラーセンサー用に値を読み込む
 */
void ColsTsk(VP_INT exinf)
{
  for (;;)
  {
    ecrobot_process_bg_nxtcolorsensor();
    dly_tsk(2);
  }
}
/*
 * 周期タイマ
 *	タスクを定期的に起動するだけ
 */
void MoveCyc(VP_INT exinf)
{
  isig_sem(Stskc); // MoveTskを進めるためにセマフォを操作
}

void DispCyc(VP_INT exinf)
{
  iact_tsk(Tdisp); // DispTskを定期的に起動
}

/* OSにより1msごとに呼び出される */
void jsp_systick_low_priority(void)
{
  if (get_OS_flag())
  {
    isig_tim(); // 今回はタイマを使っているのでこの呼び出しが必要
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
  ecrobot_init_sonar_sensor(Sonar);
}

/* システム停止時に呼ばれるルーチン */
void ecrobot_device_terminate(void)
{
  nxt_motor_set_speed(Rmotor, 0, 1);
  nxt_motor_set_speed(Lmotor, 0, 1);
  nxt_motor_set_speed(Amotor, 0, 1);
  ecrobot_term_nxtcolorsensor(Color);
  //ecrobot_set_light_sensor_inactive(Light);
  ecrobot_term_sonar_sensor(Sonar);
}
