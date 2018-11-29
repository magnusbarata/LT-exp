/*
 * Color Sensing algorithm collection
 */

/* Color Info
 * R: {598, 296, 223}, G: {381, 481, 263}, B: {355, 324, 360}
 * C: {317, 474, 461}, M: {594, 312, 337}, Y: {641, 570, 350}
 * W: {}, K: {296, 243, 189}
 */

 //---------- High-Low ----------//
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
         display_string("RED"); break;
       case 5:
         display_string("MAGENTA"); break;
       case 6:
         display_string("YELLOW"); break;
       case 7:
         display_string("WHITE"); break;
     }
     display_update();
   }
 }

//---------- High-Mid-Low ----------//
//---------- Bigger-Smaller ----------//
if (red < blue && red < green && red < 20)
{
 Serial.println(" - (Red Color)");
 digitalWrite(redLed, HIGH); // Turn RED LED ON
 digitalWrite(greenLed, LOW);
 digitalWrite(blueLed, LOW);
}

else if (blue < red && blue < green)
{
 Serial.println(" - (Blue Color)");
 digitalWrite(redLed, LOW);
 digitalWrite(greenLed, LOW);
 digitalWrite(blueLed, HIGH); // Turn BLUE LED ON
}

else if (green < red && green < blue)
{
 Serial.println(" - (Green Color)");
 digitalWrite(redLed, LOW);
 digitalWrite(greenLed, HIGH); // Turn GREEN LED ON
 digitalWrite(blueLed, LOW);
}  
