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

    motor_set_speed(Amotor, 0, 1);
  }
