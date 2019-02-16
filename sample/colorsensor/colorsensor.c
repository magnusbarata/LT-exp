/* colorsensor value check program */

#include <t_services.h>
#include "ecrobot_base.h"
#include "ecrobot_interface.h"

#define Color NXT_PORT_S4

// main program
void
MainTsk(VP_INT exinf)
{
	S16 rgb[3];

	ecrobot_set_nxtcolorsensor(Color, NXT_COLORSENSOR);
	dly_tsk(1000);
	for (;;) {
		ecrobot_get_nxtcolorsensor_rgb(Color, rgb);
		display_clear(0);
		display_goto_xy(2, 2);
		display_string("R: ");
		display_int(rgb[0], 4);
		display_goto_xy(2, 3);
		display_string("G: ");
		display_int(rgb[1], 4);
		display_goto_xy(2, 4);
		display_string("B: ");
		display_int(rgb[2], 4);
		display_update();
		dly_tsk(2000);
	}
}

// Colorsensor value update routine
void
ColsTsk(VP_INT exinf)
{
	for (;;) {
		ecrobot_process_bg_nxtcolorsensor();
	}
}

// NXT-JSP hook ISR to be invoked every 1msec
void
jsp_systick_low_priority(void)
{
	if (get_OS_flag()) {
		isig_tim();
		check_NXT_buttons();
	}
}

// System Initialize Routine
void
ecrobot_device_initialize(void)
{
	ecrobot_init_nxtcolorsensor(Color, NXT_COLORSENSOR);
}

// System Shutdown Routine
void
ecrobot_device_terminate(void)
{
	ecrobot_term_nxtcolorsensor(Color);
}
