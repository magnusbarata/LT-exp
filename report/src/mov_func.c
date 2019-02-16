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
    // PID制御
    Ldeg = nxt_motor_get_count(Lmotor);
    Rdeg = nxt_motor_get_count(Rmotor);
    cur_err = Ldeg-Rdeg-RATIO;
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
  motor_set_speed(Lmotor, 0, 1);
  motor_set_speed(Rmotor, 0, 1);
}
