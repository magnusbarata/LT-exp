/* 外部変数の定義 */
U8 rtouch;  //if 1 = on, 0 = off
U8 ltouch;  //if 1 = on, 0 = off

/* 作成した関数 */
int Color_func(void);
int R_Touch_func(void);
int L_Touch_func(void);

/* Displaying Sensor Output */
U8 bin(const int val, const int div, const int n){
    if(val > div) return 1 << n;
    return 0 << n;
}

int Color_func(void){
    S16 col[3];
    U8 bits = 0;
    
    // Read Color value
    ecrobot_get_nxtcolorsensor_rgb(Color, col);
    bits = bin(col[0], 400, 2) |
    bin(col[1], 350, 1) |
    bin(col[2], 320, 0);
    
    return bits;
}

/* Rightタッチセンサー */
int R_Touch_func(void){
    return ecrobot_get_touch_sensor(Rtouch);  //if 1 = on, 0 = off
}

/* Leftタッチセンサー */
int L_Touch_func(void){
    return ecrobot_get_touch_sensor(Ltouch); //if 1 = on, 0 = off
}

void algorithm_collect{
    int lspeed,rspeed;
    
    ecrobot_set_nxtcolorsensor(Color, NXT_COLORSENSOR);
    
    lspeed = rspeed = 20;
    motor_set_speed(Rmotor, rspeed, 1);
    motor_set_speed(Lmotor, lspeed, 1);
    if(flag1 == 1 && flag2 == 1){
        rspeed = lspeed = -20;
    }
    
}
