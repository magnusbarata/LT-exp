/*
 * Color Sensing algorithm collection
 */

 //---------- High-Low ----------//
 void dispTouch_func(void){
   // Touch sensor display
   U8 flag = ecrobot_get_touch_sensor(U8 port_id);

 }


 U8 bin(const int val, const int div, const int n){
   if(val > div) return 1 << n;
   return 0 << n;
 }

 void dispColor_func(void){
   S16 col[3];
   U8 bits = 0;

   ecrobot_set_nxtcolorsensor(Color, NXT_COLORSENSOR);
   display_clear(0);
   for(;;){
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
