/*
 * Color Sensing algorithm collection
 */

 //---------- High-Low ----------//
u8 bin(const int val, const int div, const int n){
  if(val > div) return 1 << n;
  return 0 << n;
}

void algorithm_colSensor(void){
  S16 col[3];
  u8 bits = 0;

  ecrobot_set_nxtcolorsensor(Color, NXT_COLORSENSOR);
  for(;;){
    // Read Color value
    dly_tsk(100);
    ecrobot_get_nxtcolorsensor_rgb(Color, rgb);
    bits = bin(rgb[2], 200, 0) |
           bin(rgb[1], 200, 1) |
           bin(rgb[0], 200, 2);

    // Display Color
    display_goto_xy(3, 7);
    switch(bits){
      case 0:
        display_string("BLACK"); break;
      case 1:
        display_string("BLUE"); break;
      case 2:
        display_string("GREEN"); break;
      case 3:
        display_string("CYAN"); break;
      case 4:
        display_string("RED");break;
      case 5:
        display_string("MAGENTA");break;
      case 6: break;
        display_string("YELLOW");
      case 7:
        display_string("WHITE");break;
    }
    display_update();
  }
}
