/*
 *	TOPPERS/JSPを用いたライントレーサーのサンプルコード
 */

/*
  TODO:
    calibration(), collect_all()
  EXP:
    Motor, Arm, Display, Sound, Collect All
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
#define REVERSE

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
void collect_red_ball(void);
void calibration(void);
void alg_collect_all(void);
void alg_collect_red_ball(void);

/* 外部変数の定義 */
char name[17];
void (*algorithm)(void) = calibration;

/*----------- 雑多な関数群 -----------*/
void wait_for_release(void)
{
  while (ecrobot_get_button_state()) {
    dly_tsk(7);
    ecrobot_poll_nxtstate();
  }
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

U8 bin(const int val, const int div, const int n)
{
  if(val > div) return 1 << n;
  return 0 << n;
}

int spd_limit(const int val){
  if(val > 20) return 20;
  else if(val < -20) return -20;
  else return val;
}

void mov_func(const int POW, int RATIO, const int DEG)
{
  //正：右，負：左
  int Ldeg, Rdeg, turn;
  int cur_err, prev_err, integral, derivative;
  double kp = 70.0;
  double ki = 0;
  double kd = 0;

  nxt_motor_set_count(Lmotor, 0);
  nxt_motor_set_count(Rmotor, 0);
  prev_err = integral = derivative = 0;

  do{
    //wai_sem(Stskc);

    // PID制御
    Ldeg = nxt_motor_get_count(Lmotor);
    Rdeg = nxt_motor_get_count(Rmotor);
    cur_err = Ldeg-Rdeg-RATIO;
    //cur_err = Ldeg*RATIO - Rdeg*(1.0-RATIO);  // double, 0の時0にならないよう
    integral = integral + cur_err;
    derivative = cur_err - prev_err;
    turn = kp * cur_err + ki * integral + kd * derivative;
    motor_set_speed(Lmotor, POW - spd_limit(turn), 1); // -spd_limit(turn)?
    motor_set_speed(Rmotor, POW + spd_limit(turn), 1);
    prev_err = cur_err;

    // モーター角度表示
    display_goto_xy(1, 2);
    display_string("Lmotor:"); display_int(Ldeg, 4);
    display_goto_xy(1, 3);
    display_string("Rmotor:"); display_int(Rdeg, 4);
    display_update();
    if(POW < 0 && Ldeg <= DEG) break;
    else if(POW > 0 && Ldeg >= DEG) break;
    } while(1);
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
if(DEG < 0)POW = -2*POW;
 while(1){ // <= OR >= (?)
    wai_sem(Stskc);
    Adeg = nxt_motor_get_count(Amotor);
    if(DEG < 0 && Adeg <= DEG) break;
    else if(DEG > 0 && Adeg >= DEG) break;
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
  {"Calibration", calibrate},
  {"Collect All", collect_all},
  {"Collect Red Ball", collect_red_ball},
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
  act_tsk(Tmain);
}

void alg_collect_blue_ball() {

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


void calibrate(void) {algorithm = calibration;}
void calibration(void)
{
  // アーム上げる
  arm_func(20, -30);
  mov_func(-50, 0, -1800);
  // アーム下げる
  arm_func(20, 30);
  //右寄りに進む
  mov_func(-50, -20, -2400);
  mov_func(50, 0, 180);
  arm_func(20, -50);
  mov_func(50, 0, 1000);
  mov_func(-50, 360, -500);
  mov_func(-50, 360, -500);
 mov_func(-50, 360, -500);
 mov_func(-50, 0, -2200);
  // ピンクエリアに設置
  //arm_func(30, -60);
  // ちょっと押す
  //mov_func(-50, 0, -100);
  
  //mov_func(70, 0, 2000);
  //mov_func(-70, 200, -8000);
  //mov_func(-70, 0, -4000);

  // TO TEST
  // arm_func(10, 30); // アームを下げる
  // arm_func(10, -30); // アームを上げる
  // mov_func(-40, 0, -700); // 直進
  // mov_func(40, 0, 700); // 直進
  //  mov_func(-40, 20, -3000); //　左曲がり
  // mov_func(-40, -20, -3000); // 右曲がり

  // mov_func(-100, 0, 1000);
  //arm_func(10, -30); // アームを下げる
  // arm_func(20, 30); // アームを上げる
  // mov_func(100, 0, 1000);


}

/*----------- アルゴリズム群 -----------*/
void collect_all(void) {algorithm = alg_collect_all;}
void alg_collect_all(void)
{
  //arm_func(10, 30);
}

void collect_red_ball(void) {algorithm = alg_collect_red_ball;}
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
		wai_sem(Stskc);

    // カラーセンサー
    ecrobot_get_nxtcolorsensor_rgb(Color, col);
    CBits = bin(col[0], COL_THRES[0], 2) |
            bin(col[1], COL_THRES[1], 1) |
            bin(col[2], COL_THRES[2], 0);
    // フラッグをクリアしてからセットする
    //clr_flg(Fsens, ~(BLK | BLU | GRN | CYA |
    //                 RED | MAG | YEL | WHT)); (?)
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
		} else {
      set_flg(Fsens, RTR);
    }
		if (ecrobot_get_touch_sensor(Ltouch)) {
			set_flg(Fsens, LTP);
		} else {
      set_flg(Fsens, LTR);
    }
	}

  sig_sem(Stskc);
}

void NbtnTsk(VP_INT exinf)
{
  U8 btn;

  for (;;){
    wai_sem(Snbtn);
    btn = ecrobot_get_button_state();
    switch(btn){
      case Obtn: set_flg(Fnbtn, Obtn); break;
      case Lbtn: set_flg(Fnbtn, Lbtn); break;
      case Rbtn: set_flg(Fnbtn, Rbtn); break;
      case Cbtn: set_flg(Fnbtn, Cbtn); break;
      default: ecrobot_poll_nxtstate(); break;
    }
    sig_sem(Snbtn);
    dly_tsk(10);
  }
}

void QuitTsk(VP_INT exinf)
{
  //FLGPTN BtnSens;

  for (;;) {
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
    if (ecrobot_is_ENTER_button_pressed()) {
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

  sta_cyc(Cmove);
  //sta_cyc(Cdisp);
  (*algorithm)();
}

void MoveTsk(VP_INT exinf)
{
  FLGPTN MtrSens;

  for(;;){
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

  for(;;){
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
  display_goto_xy(0, 7);
  //ref_flg(Fsens, &sens);
  wai_flg(Fsens, BLK | BLU | GRN | CYA | RED |
                 MAG | YEL | WHT | RTP | LTP |
                 RTR | LTR | POS | DIS, TWF_ORW, &sens);

  switch(sens){
    // カラー表示
    case BLK: display_string("K"); break;
    case BLU: display_string("B"); break;
    case GRN: display_string("G"); break;
    case CYA: display_string("C"); break;
    case RED: display_string("R"); break;
    case MAG: display_string("M"); break;
    case YEL: display_string("Y"); break;
    case WHT: display_string("W"); break;

    // タッチ表示
  case LTP:
      display_goto_xy(1, 7); display_string("[");
      break;
  case RTP:
      display_goto_xy(2, 7); display_string("]");
      break;
      // case (LTP & LTR) | (RTP & RTR):
      //display_goto_xy(1, 7); display_string("[]");
      //break;

    // モーターとアーム
    case POS:
      display_goto_xy(3, 7); display_string("P");
      break;
    case DIS:
      display_goto_xy(4, 7); display_string("D");
      break;

    default:
      display_goto_xy(1, 7); display_string("----");
      break;
  }

  display_update();
}

void MuscTsk(VP_INT exinf)
{
  FLGPTN ColSens;

  for (;;) {
      wai_flg(Fsens, BLK | BLU | GRN | CYA | RED |
                 MAG | YEL | WHT | RTP | LTP |
	      RTR | LTR | POS | DIS, TWF_ORW, &ColSens);

    // 色ごとに音が変わる(C4からC5まで)
    switch(ColSens){
      case BLK: ecrobot_sound_tone(262, 100, 70); break;
      case BLU: ecrobot_sound_tone(294, 100, 70); break;
      case GRN: ecrobot_sound_tone(330, 100, 70); break;
      case CYA: ecrobot_sound_tone(349, 100, 70); break;
      case RED: ecrobot_sound_tone(392, 100, 70); break;
      case MAG: ecrobot_sound_tone(440, 100, 70); break;
      case YEL: ecrobot_sound_tone(494, 100, 70); break;
      case WHT: ecrobot_sound_tone(523, 200, 0); break;
    }
    //dly_tsk(10);

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
