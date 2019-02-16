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
