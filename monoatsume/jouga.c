/* 20190124
 *	TOPPERS/JSPを用いたライントレーサーのサンプルコード
 */

/*
  TODO:
    calibration(), collect_all()
  EXP:
    Motor, Arm, Display, Sound, Collect All
*/
#include "display.h" // 変更したバージョンを使うために先頭でinclude

#include "kernel_id.h"
#include "ecrobot_interface.h"
#include "ecrobot_base.h"

#include "jouga_cfg.h"
#include "jouga.h"
#include "music.h"
#include "graphics.h"

#include <t_services.h>
#include "ecrobot_interface.h"
#include "mytypes.h" // U8の定義が必要
#define REVERSE

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

typedef enum
{
  Obtn = 1 << 0,
  Lbtn = 1 << 1,
  Rbtn = 1 << 2,
  Cbtn = 1 << 3,
} Nbtns;

U8 ecrobot_get_button_state(void);
void ecrobot_poll_nxtstate(void);

void calibrate(void);
void collect_red_ball(void);
void calibration(void);
void alg_collect_red_ball(void);

/*------Suzu Algorithm---------------*/
void collect_BlueStart();
void collect_GreenStart();
void collect_BlueFinish();
void collect_GreenFinish();
void move_wheel();
void set_tBlock();
void alg_collect_BlueStart();
void alg_collect_GreenStart();
void alg_collect_BlueFinish();
void alg_collect_GreenFinish();
void alg_move_wheel();
void alg_set_tBlock();

/* 外部変数の定義 */
char name[17];
void (*algorithm)(void) = collect_BlueStart;

/*----------- 雑多な関数群 -----------*/
void wait_for_release(void)
{
  while (ecrobot_get_button_state())
  {
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  }
}

U8 get_btn(void)
{
  U8 btn, t;

  do
  {
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  } while (!(btn = ecrobot_get_button_state()));
  while ((t = ecrobot_get_button_state()))
  {
    btn |= t;
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  }
  return btn;
}

U8 bin(const int val, const int div, const int n)
{
  if (val > div)
    return 1 << n;
  return 0 << n;
}

int spd_limit(const int val)
{
  if (val > 20)
    return 20;
  else if (val < -20)
    return -20;
  else
    return val;
}
void mov_func(const int POW, int RATIO, const int DEG)
{
  //正：右，負：左
  int Ldeg, Rdeg, turn;
  int cur_err, prev_err, integral, derivative;
  double kp = 90.0; // 70.0
  double ki = 0;
  double kd = 0;

  nxt_motor_set_count(Lmotor, 0);
  nxt_motor_set_count(Rmotor, 0);
  prev_err = integral = derivative = 0;

  do
  {
    //wai_sem(Stskc);

    // PID制御
    Ldeg = nxt_motor_get_count(Lmotor);
    Rdeg = nxt_motor_get_count(Rmotor);
    cur_err = Ldeg - Rdeg - RATIO;
    //cur_err = Ldeg*RATIO - Rdeg*(1.0-RATIO);  // double, 0の時0にならないよう
    integral = integral + cur_err;
    derivative = cur_err - prev_err;
    turn = kp * cur_err + ki * integral + kd * derivative;
    motor_set_speed(Lmotor, POW - spd_limit(turn), 1); // -spd_limit(turn)?
    motor_set_speed(Rmotor, POW + spd_limit(turn), 1);
    prev_err = cur_err;

    // モーター角度表示
    display_goto_xy(1, 2);
    display_string("Lmotor:");
    display_int(Ldeg, 4);
    display_goto_xy(1, 3);
    display_string("Rmotor:");
    display_int(Rdeg, 4);
    display_update();
    if (POW < 0 && Ldeg <= DEG)
      break;
    else if (POW > 0 && Ldeg >= DEG)
      break;
  } while (1);
  //set_flg(Fsens, DIS);
  motor_set_speed(Lmotor, 0, 1);
  motor_set_speed(Rmotor, 0, 1);
}

void arm_func(int POW, const int DEG)
{
  // TODO: 上げ下げのオプション
  // 正：下，負：上
  int Adeg;
  nxt_motor_set_count(Amotor, 0);
  if (DEG < 0)
    POW = -2 * POW;
  while (1)
  { // <= OR >= (?)
    wai_sem(Stskc);
    Adeg = nxt_motor_get_count(Amotor);
    if (DEG < 0 && Adeg <= DEG)
      break;
    else if (DEG > 0 && Adeg >= DEG)
      break;
    motor_set_speed(Amotor, POW, 1);

    // アーム角度表示
    display_goto_xy(1, 1);
    display_string("Arm:");
    display_int(nxt_motor_get_count(Amotor), 4);
    display_update();
  }
  //set_flg(Fsens, POS);
  motor_set_speed(Amotor, 0, 1);
}

/*----------- メニュー -----------*/
NameFunc MainMenu[] = {
    {"Main Menu", NULL},
    {"Start", NULL},
    //{"Calibration", calibrate},
    {"Collect BlueStart", collect_BlueStart},
    //  {"Collect GreenStart", collect_GreenStart},
    // {"Collect BlueFinish", collect_BlueFinish},
    // {"Collect GreenFinish", collect_GreenFinish},
    //  {"Move Wheel", move_wheel},
    {"Set TBlock", set_tBlock},
    {"Exit", ecrobot_restart_NXT}, // OSの制御に戻る
                                   //  {"Power Off", ecrobot_shutdown_NXT},	// 電源を切る
};

/* メニューを表示して選択されるのを待つ */
void func_menu(NameFunc *tbl, int cnt)
{
  int i;
  static int menu = 1;
  U8 btn;

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
  act_tsk(Tmain);
}

void alg_collect_blue_ball()
{

  // hardCoding
  // とりあえず前進

  mov_func(-LOWPOWER, 0, -1500);
  arm_func(-LOWPOWER, 500);

  // アームを上げた状態で少し前進
  mov_func(LOWPOWER, 0, 360);
  // アームを下げる
  arm_func(LOWPOWER, 500);
  // バック

  // 左に曲がる
  mov_func(LOWPOWER, -45, 700);
  // 右に曲がりながらピンクエリアへ向かう(このぐらいの曲がり具合なら途中で壁に当たればピンクエリアへ向かう)
  mov_func(HIGHPOWER, 10, 1500);

  // アーム上げ下げしてオブジェクトを置く
  arm_func(-LOWPOWER, 500);
  // バック
  mov_func(-HIGHPOWER, 0, 2000);
  act_tsk(Tmotr);
}

void steering(int direction, float angle)
{
  // direction (-1 : 左 | 1 : 右)
  // angle (ステアリング角度の絶対値)

  // 動かすモーターの回転数を格納
  int moving_motor_count = 0;
  // 動かすモーターと止めるモーターのポート
  const int moving_motor = (direction == 1) ? Lmotor : Rmotor;
  const int stopping_motor = (direction == 1) ? Rmotor : Lmotor;

  // 初期化
  nxt_motor_set_count(Rmotor, 0);
  nxt_motor_set_count(Lmotor, 0);
  // 旋回開始
  nxt_motor_set_speed(moving_motor, -75, 0);
  nxt_motor_set_speed(stopping_motor, 0, 1);
  while (moving_motor_count < angle)
  {
    moving_motor_count = -nxt_motor_get_count(moving_motor);
  }
  // 旋回終了
  nxt_motor_set_speed(moving_motor, 0, 1);
}

/*---------------------------Start関数群-------------------------*/
void collect_GreenStart()
{
  algorithm = alg_collect_GreenStart;
}
void alg_collect_GreenStart()
{
  /*-----------------------------緑エリアスタートver-----------------------------------------*/
  // 1個目
  mov_func(-50, 0, -390);
  mov_func(-50, -310, -305);
  mov_func(-50, 0, -110); // Move until right touch is pressed
  //while (ecrobot_get_touch_sensor(Rtouch) == 0) mov_func(-50, 0, -5);
  //mov_func(-50, 0, -5);
  mov_func(-50, 310, -147); // Prev:(-50, 320, -150), Shove the ball on the left

  // 2個目
  mov_func(50, 0, 510);
  arm_func(12, -1);
  steering(-1, 360 * 0.7); // Aiming ball on left
  mov_func(-50, 0, -700); // Prev:(-50, 0, -680), Forward until first red ball
  //while (ecrobot_get_touch_sensor(Rtouch) == 0) mov_func(-50, 0, -5);
  mov_func(-50, 0, -13);

  // 3個目
  mov_func(50, 0, 350);
  steering(-1, 360 * 0.7); // Avoiding middle wheel
  mov_func(-50, -180, -700);
  mov_func(-50, 0, -400);
  steering(1, 360 * 0.3); // Aiming ball on the right
  mov_func(-50, 0, -40); // Prev:(-50, 0, -30), Shove the ball, right_touch?

  // 4個目
  mov_func(50, 0, 200); // Prev:(50, 0, 180)
  mov_func(-50, 100, -380); // Prev:(-50, 100, -400)
  mov_func(-50, -300, -500); // Prev:(-50, -300, -600)

  ///// Addition1
   mov_func(-50, 90, -350);
   mov_func(-50, 0, -100); // Forward until wall, all_touch?
   //while (ecrobot_get_touch_sensor(Rtouch) == 0) mov_func(-50, 0, -5);

  // Put all balls
  mov_func(50, 0, 8);
  mov_func(-50, 0, -4); mov_func(-50, 0, -4); // Shake!
  arm_func(12, -40);
  mov_func(50, 0, 360); // reverse a bit

  // Going back to base
  steering(-50, 360 * 1.5);
  mov_func(-50, 600, 750);
  mov_func(-100, 0, -2000);  // All power straight to base
}
void collect_BlueStart()
{
  algorithm = alg_collect_BlueStart;
}
void alg_collect_BlueStart()
{
  // 青スタートの場合は1, 緑エリアスタートの場合は-1
  // T字ブロックのやつとは逆
  const int startPosition = 1;
  /*------------------------------青エリアスタートver------------------------------*/

 // 1個目
  mov_func(-50, 0, -390);
  mov_func(-50, 310, -305);
  mov_func(-50, 0, -110); // Move until left touch is pressed
  while (ecrobot_get_touch_sensor(Ltouch) == 0) mov_func(-50, 0, -5);
  mov_func(-50, 0, -5);
  mov_func(-50, -310, -147); // Prev:(-50, 320, -150), Shove the ball on the left

  // 2個目
  mov_func(50, 0, 510);
  arm_func(12, -1);
  steering(1, 360 * 0.7); // Aiming ball on right
  //mov_func(-50, 0, -700); // Prev:(-50, 0, -680), Forward until first red ball
  while (ecrobot_get_touch_sensor(Ltouch) == 0) mov_func(-50, 0, -5);
  mov_func(-50, 0, -5);
  mov_func(-50, 0, -13);

  // 3個目
  mov_func(50, 0, 350);
  steering(1, 360 * 0.7); // Avoiding middle wheel
  mov_func(-50, 180, -700);
  mov_func(-50, 0, -400);
  steering(-1, 360 * 0.4); // Aiming ball on the right
  mov_func(-50, 0, -70); // Prev:(-50, 0, -30), Shove the ball, left_touch?

  // 4個目
  mov_func(50, 0, 200); // Prev:(50, 0, 180)
  mov_func(-50, -100, -380); // Prev:(-50, 100, -400)
  mov_func(-50, 300, -500); // Prev:(-50, -300, -600)

  ///// Addition1
   mov_func(-50, -90, -350);
   //mov_func(-50, 0, -100); // Forward until wall, all_touch?
   while (ecrobot_get_touch_sensor(Ltouch) == 0) mov_func(-50, 0, -5);

  // Put all balls
  mov_func(50, 0, 8);
  mov_func(-50, 0, -4); mov_func(-50, 0, -4); // Shake!
  arm_func(12, -30);
  mov_func(50, 0, 360); // reverse a bit

  // Going back to base
  steering(1, 360 * 1.5);
  mov_func(-50, -600, 750);
  mov_func(-100, 0, -2000);  // All power straight to base
}

/*--------------------------2重タイヤを黄色エリアへ-------------------------*/
void move_wheel()
{
  algorithm = alg_move_wheel;
}
// タイヤを青エリアに入らないようなところに設置
void alg_move_wheel(const int startPosition)
{
  mov_func(-50, 0, -350);
  steering(-1 * startPosition, 360);

  arm_func(20, -30);
  // タイヤへ突進し, 回収
  mov_func(-50, 0, -810);
  // 少し前進し, アームを下げて回収
  // mov_func(-50, 0, -120);
  arm_func(20, 45);
  // 元の場所より少し奥へ後退
  mov_func(50, 0, 700);
  // 黄色エリアへ狙いを定める
  // steering(1 * startPosition, 360 * 0.95);
  steering(1 * startPosition, 360 * 1.45);
  mov_func(-50, 0, -535);
  mov_func(50, 0, 25);
  // アームを上げてタイヤを放つ
  arm_func(20, -30);
  // ちょっと, 押してタイヤを奥の方へしまう.
  mov_func(-50, 0, -100);
  // 一時帰宅
  mov_func(50, 0, 600);
  steering(1 * startPosition, 360);
  mov_func(-50, 0, -1000);
}
/*---------------------------T字ブロックを設置-------------------------*/
void set_tBlock()
{
  algorithm = alg_set_tBlock;
}
void alg_set_tBlock()
{
  // 青スタートの場合は-1, 緑エリアスタートの場合は1
  const int startPosition = -1;
  // カラーセンサーの閾値
  static int COL_THRES[] = {400, 350, 320};
  // nxtcolorsensor_rgbを使って得られる色を格納する配列
  S16 col[3];
  // 色を識別する(この関数内では6となってればよい)
  // マゼンタの時は5になってればよい
  U8 CBits = 0;

  // 各種PID制御変数
  int Ldeg, Rdeg, turn;
  int cur_err, prev_err, integral, derivative;
  double kp = 70.0;
  double ki = 0;
  double kd = 0;

  alg_move_wheel(startPosition);
  while (1)
  {
    check_NXT_buttons();
    // ベースエリア内で設置してRUNボタンが押されたら, 発進
    if (ecrobot_is_RUN_button_pressed() == 1)
    {
      break;
    }
  }

  arm_func(20, 40);
  // エリアから出て中央方向へ狙いを定める
  mov_func(-50, 0, -200);
  steering(-1 * startPosition, 360);

  mov_func(-50, 0, -950);
  // 90°旋回してT字ブロックへ狙いを定める
  steering(1 * startPosition, 360 * 1.1);
  //  steering(-1 * startPosition, 360 * 0.1);
  arm_func(10, -5);
  mov_func(-50, 0, -295);
  // アームを上げてT字ブロック回収
  arm_func(10, -15);
  mov_func(50, 0, 200);
  steering(1 * startPosition, 360 * 0.7);
  // 黄色エリアへ狙いを定めたので直進
  mov_func(-50, 0, -1100);

  // とりあえず止まる
  mov_func(50, 0, 50);
  // T字ブロック設置
  arm_func(15, 30);
  // 帰宅
  mov_func(50, 0, 500);
  steering(1 * startPosition, 360);
  mov_func(-70, 50 * startPosition, -1500);
}

/*---------------------------Finish関数群-------------------------*/
// オブジェクトを持った状態で自陣地に機体がある時に押せば斜め奥のエリアにオブジェクトを置くアルゴリズム
void collect_BlueFinish()
{
  algorithm = alg_collect_BlueFinish;
}
void alg_collect_BlueFinish()
{

  /*---------------青エリアFinishVer---------------*/
  mov_func(-50, 20, -3000);
  // 1歩下がってアーム上げ下げしてボール取得
  mov_func(50, 0, 100);
  arm_func(20, -30);
  mov_func(-50, 0, -100);
  mov_func(50, 0, 100);

  // 帰宅
  mov_func(50, 0, 500);
  steering(1, 360 * 5);
  mov_func(-50, 0, -1500);
}

void collect_GreenFinish()
{
  algorithm = alg_collect_GreenFinish;
}
void alg_collect_GreenFinish()
{
  /*-----------------緑エリアFinishVer------------*/
  mov_func(-50, -20, -3000);
  // 1歩下がってアーム上げ下げしてボール取得
  mov_func(50, 0, 100);
  arm_func(20, -30);
  mov_func(-50, 0, -100);
  mov_func(50, 0, 100);

  // 帰宅
  mov_func(50, 0, 500);
  steering(-1, 360 * 5);
  mov_func(-50, 0, -1500);
}

void calibrate(void)
{
  algorithm = calibration;
}
void calibration(void)
{
  /*Reference*/
  /*
右曲がり : mov_func(-50, -30, 100);
左曲がり : mov_func(-50, 30, 100);
アーム上げ : arm_func(20, -30);
アーム下げ : arm_func(20, 30);
左旋回 : steering(-1, 360 * 3);
右旋回 : steering(1, 360 * 3);
*/
}

/*----------- アルゴリズム群 -----------*/
void collect_red_ball(void)
{
  algorithm = alg_collect_red_ball;
}
void alg_collect_red_ball(void)
{
}

/*----------- タスク群 -----------*/
void SensTsk(VP_INT exinf)
{
  static int COL_THRES[] = {400, 350, 320};
  S16 col[3];
  U8 CBits = 0;

  for (;;)
  {
    wai_sem(Stskc);

    // カラーセンサー
    ecrobot_get_nxtcolorsensor_rgb(Color, col);
    CBits = bin(col[0], COL_THRES[0], 2) |
            bin(col[1], COL_THRES[1], 1) |
            bin(col[2], COL_THRES[2], 0);
    // フラッグをクリアしてからセットする
    //clr_flg(Fsens, ~(BLK | BLU | GRN | CYA |
    //                 RED | MAG | YEL | WHT)); (?)
    switch (CBits)
    {
    case 0:
      set_flg(Fsens, BLK);
      break;
    case 1:
      set_flg(Fsens, BLU);
      break;
    case 2:
      set_flg(Fsens, GRN);
      break;
    case 3:
      set_flg(Fsens, CYA);
      break;
    case 4:
      set_flg(Fsens, RED);
      break;
    case 5:
      set_flg(Fsens, MAG);
      break;
    case 6:
      set_flg(Fsens, YEL);
      break;
    case 7:
      set_flg(Fsens, WHT);
      break;
    }

    // タッチセンサー
    if (ecrobot_get_touch_sensor(Rtouch))
    {
      set_flg(Fsens, RTP);
    }
    else
    {
      set_flg(Fsens, RTR);
    }
    if (ecrobot_get_touch_sensor(Ltouch))
    {
      set_flg(Fsens, LTP);
    }
    else
    {
      set_flg(Fsens, LTR);
    }
  }

  sig_sem(Stskc);
}

void NbtnTsk(VP_INT exinf)
{
  U8 btn;

  for (;;)
  {
    wai_sem(Snbtn);
    btn = ecrobot_get_button_state();
    switch (btn)
    {
    case Obtn:
      set_flg(Fnbtn, Obtn);
      break;
    case Lbtn:
      set_flg(Fnbtn, Lbtn);
      break;
    case Rbtn:
      set_flg(Fnbtn, Rbtn);
      break;
    case Cbtn:
      set_flg(Fnbtn, Cbtn);
      break;
    default:
      ecrobot_poll_nxtstate();
      break;
    }
    sig_sem(Snbtn);
    dly_tsk(10);
  }
}

void QuitTsk(VP_INT exinf)
{
  //FLGPTN BtnSens;

  for (;;)
  {
    /*wai_flg(Fnbtn, Obtn, TWF_ORW, &BtnSens);
    if(BtnSens){ // (BtnSens == Obtn) OR case?
      nxt_motor_set_speed(Rmotor, 0, 0);
      nxt_motor_set_speed(Lmotor, 0, 0);
      nxt_motor_set_speed(Amotor, 0, 1);
      stp_cyc(Cmove);
      stp_cyc(Cdisp);
      ter_tsk(Tinit);
      ter_tsk(Tsens);
      ter_tsk(Tnbtn);
      ter_tsk(Tmove);
      ter_tsk(Tmotr);
      ter_tsk(Ttimr);
      ter_tsk(Tmusc);
      ter_tsk(Tmain);
      ter_tsk(Tdisp);
      act_tsk(Tinit);
      clr_flg(Fsens, ~(Obtn | Lbtn | Rbtn | Cbtn));
    }*/
    wai_sem(Snbtn);
    check_NXT_buttons();
    if (ecrobot_is_ENTER_button_pressed())
    {
      nxt_motor_set_speed(Rmotor, 0, 0);
      nxt_motor_set_speed(Lmotor, 0, 0);
      nxt_motor_set_speed(Amotor, 0, 1);
      stp_cyc(Cmove);
      stp_cyc(Cdisp);
      ter_tsk(Tinit);
      ter_tsk(Tsens);
      ter_tsk(Tnbtn);
      ter_tsk(Tmove);
      ter_tsk(Tmotr);
      ter_tsk(Ttimr);
      ter_tsk(Tmusc);
      ter_tsk(Tmain);
      ter_tsk(Tdisp);
      act_tsk(Tinit);
    }
    sig_sem(Snbtn);
    dly_tsk(5);
    ter_tsk(Tquit);
  }
}

void InitTsk(VP_INT exinf)
{
  display_clear(0); // なにはともあれ、画面をクリア
  display_goto_xy(2, 3);
  display_string("Initializing");
  display_update();
  ecrobot_get_bt_device_name(name); // システム名の取得
  act_tsk(Tsens);
  act_tsk(Tnbtn);
  act_tsk(Tdisp);
  act_tsk(Tmove);

  // ボタンが押されていない状態になるまで待つ
  wait_for_release();
  wai_sem(Snbtn);                           // ボタンに関する権利を取得
  func_menu(MainMenu, ARRAYSIZE(MainMenu)); // メインメニューの表示
  sig_sem(Snbtn);                           // ボタンに関する権利を開放
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

  sta_cyc(Cmove);
  //sta_cyc(Cdisp);
  (*algorithm)();
}

void MoveTsk(VP_INT exinf)
{
  FLGPTN MtrSens;

  for (;;)
  {
    /*wai_flg(Fsens, DIS, TWF_ORW, &MtrSens);
    switch(MtrSens){
    case(DIS):
      motor_set_speed(Lmotor, 0, 1);
      motor_set_speed(Rmotor, 0, 1);
    }
    clr_flg(Fsens, ~DIS);*/
  }
}

void MotrTsk(VP_INT exinf)
{
  FLGPTN ArmSens; // twai_flg() OR ref_flg()?

  for (;;)
  {
    /*wai_flg(Fsens, POS, TWF_ANDW, &ArmSens); // TWF_ORW | TWF_CLR
    if(ArmSens == POS) motor_set_speed(Amotor, 0, 1);
    //else continue;?
    /* switch(ArmSens){
      case POS: motor_set_speed(Amotor, 0, 1); break;
    }
    clr_flg(Fsens, ~POS);*/
  }
}

void TimrTsk(VP_INT exinf)
{
  static unsigned int s = 0;
  for (;;)
  {
    // 10秒ごとに音鳴らす
    dly_tsk(1000);
    s++;
    if (!(s % 10))
      ecrobot_sound_tone(220, 100, 60);
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
  display_goto_xy(0, 7);
  //ref_flg(Fsens, &sens);
  wai_flg(Fsens, BLK | BLU | GRN | CYA | RED | MAG | YEL | WHT | RTP | LTP | RTR | LTR | POS | DIS, TWF_ORW, &sens);

  switch (sens)
  {
  // カラー表示
  case BLK:
    display_string("K");
    break;
  case BLU:
    display_string("B");
    break;
  case GRN:
    display_string("G");
    break;
  case CYA:
    display_string("C");
    break;
  case RED:
    display_string("R");
    break;
  case MAG:
    display_string("M");
    break;
  case YEL:
    display_string("Y");
    break;
  case WHT:
    display_string("W");
    break;

    // タッチ表示
  case LTP:
    display_goto_xy(1, 7);
    display_string("[");
    break;
  case RTP:
    display_goto_xy(2, 7);
    display_string("]");
    break;
    // case (LTP & LTR) | (RTP & RTR):
    //display_goto_xy(1, 7); display_string("[]");
    //break;

  // モーターとアーム
  case POS:
    display_goto_xy(3, 7);
    display_string("P");
    break;
  case DIS:
    display_goto_xy(4, 7);
    display_string("D");
    break;

  default:
    display_goto_xy(1, 7);
    display_string("----");
    break;
  }

  display_update();
}

void MuscTsk(VP_INT exinf)
{
  FLGPTN ColSens;

  for (;;)
  {
    wai_flg(Fsens, BLK | BLU | GRN | CYA | RED | MAG | YEL | WHT | RTP | LTP | RTR | LTR | POS | DIS, TWF_ORW, &ColSens);

    // 色ごとに音が変わる(C4からC5まで)
    switch (ColSens)
    {
    case BLK:
      ecrobot_sound_tone(262, 100, 70);
      break;
    case BLU:
      ecrobot_sound_tone(294, 100, 70);
      break;
    case GRN:
      ecrobot_sound_tone(330, 100, 70);
      break;
    case CYA:
      ecrobot_sound_tone(349, 100, 70);
      break;
    case RED:
      ecrobot_sound_tone(392, 100, 70);
      break;
    case MAG:
      ecrobot_sound_tone(440, 100, 70);
      break;
    case YEL:
      ecrobot_sound_tone(494, 100, 70);
      break;
    case WHT:
      ecrobot_sound_tone(523, 200, 0);
      break;
    }
    //dly_tsk(10);

    //play_notes(TIMING_chiba_univ, 8, chiba_univ);
    // TODO: 状態ごとに音変わる ()
  }
}

void ColsTsk(VP_INT exinf)
{
  for (;;)
  {
    ecrobot_process_bg_nxtcolorsensor();
    dly_tsk(2);
  }
}

/*----------- 周期 タイマー群 -----------*/
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
