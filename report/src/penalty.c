// センサーが検知した色を黒, 白, グレーで分ける
int CalcColor(const int cval, const int lval, const int range) {
  if (cval < clow + range && lval < llow + range) {
    return 0; // 黒
  }
  else if (cval > chigh - range && lval > lhigh - range) {
    return 1; // 白
  }
  else {
    return 2; //グレー
  }
}


int spd_limit(int val){
  if(val > 10) return 10;
  else if(val < -10) return -10;
  else return val;
}


void algorithm_straight(void)
{
  enum StraightState
  {
    None,
    WhiteStraight,
    ArrivedFirstCircle,
    ArrivedSecondCircle,
    StateNum,
  };


  // センサーで取得した値を元に計算した色
  enum CalculatedColor
  {
    Black,
    White,
    GRAY,
    ColorNum,
  };
  enum StraightState myState = None; // 現在のロボットの状態
  enum CalculatedColor nextColor = ColorNum;
  int range = 140; // 完全一致の黒色白色を感知するのは難しいので許容する範囲


  // 左右のモーターの回転数
  int RmotorCount = 0;
  int LmotorCount = 0;
  int subMotorCount = RmotorCount - LmotorCount;
  int isAdjusting = 0;


  int BASE_POW = 30;
  int distance = 1500, finish = 0;
  int Rdeg, Ldeg, error, turn;
  int prev_err, integral = 0, derivative = 0;
  int tol = 30;
  double kp = 100.0;
  double ki = 25;
  double kd = 0;


  /*---------------発進動作-----------------------*/
  nxt_motor_set_count(Rmotor, 0);
  nxt_motor_set_count(Lmotor, 0);

  do
  {
    wai_sem(Stskc);
    lval = get_light_sensor(Light);
    cval = get_light_sensor(Color);
    nextColor = CalcColor(cval, lval, range);

    RmotorCount = (int)nxt_motor_get_count(Rmotor) / 360;
    LmotorCount = (int)nxt_motor_get_count(Lmotor) / 360;

    if ((RmotorCount >= 6 && LmotorCount >= 6) && (myState == None))
    {
      myState = WhiteStraight;
    }
    // Calculating angle difference
    Rdeg = nxt_motor_get_count(Rmotor);
    Ldeg = nxt_motor_get_count(Lmotor);
    error = Ldeg - Rdeg;

    // PID
    integral = integral + error;
    derivative = error - prev_err;
    turn = kp * error + ki * integral + kd * derivative;

    // Control
    // motor_set_speed(Rmotor, BASE_POW+speed_limit(turn), 1);
    motor_set_speed(Rmotor, BASE_POW, 1); // Master
    motor_set_speed(Lmotor, BASE_POW - spd_limit(turn), 1); // Slave
    prev_err = error;

    // Display
    display_goto_xy(0, 0);
    display_string("LRdeg: ");
    display_int(Ldeg, 4);
    display_int(Rdeg, 4);

    switch (myState) {
      // 白領域走行
      case WhiteStraight:
      // 白領域を走行中に黒を感知したらステートを1つ目の円感知後に変更
      if (nextColor == Black)
      {
        myState = ArrivedFirstCircle;
      }
      break;

      // ↓結構ハードコーディングだけどあまりいい案が浮かなばなかった...
      // 最初の黒円到達後
      case ArrivedFirstCircle:
      dly_tsk(100);
      myState = ArrivedSecondCircle;
      break;
      // 2個目の黒円到達後
      case ArrivedSecondCircle:
      myState = None;
      motor_set_speed(Rmotor, 0, 1);
      motor_set_speed(Lmotor, 0, 1);
      break;

      default:
      break;
    }
  } while (1);
