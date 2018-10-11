/* EventFlag sample program */

#include "kernel_id.h"
#include <t_services.h>
#include "ecrobot_base.h"
#include "ecrobot_interface.h"

#include "defs.h"

typedef enum {
	eRtouch = 1 << 0,	// Touch Sensor (Right)
	eLtouch = 1 << 1,	// Touch Sensor (Left)
} EBits;

#define Rtouch NXT_PORT_S1
#define Ltouch NXT_PORT_S4

// main program
void
MainTsk(VP_INT exinf)
{
	FLGPTN	sensor;

	dly_tsk(1000);
	for (;;) {
		wai_flg(Fsens, eRtouch | eLtouch, TWF_ORW, &sensor);
		switch (sensor) {
		case eRtouch:
			ecrobot_sound_tone(440, 100, 60);
			break;
		case eLtouch:
			ecrobot_sound_tone(880, 100, 60);
			break;
		case eRtouch | eLtouch:
			ecrobot_sound_tone(220, 100, 60);
			break;
		}
		dly_tsk(1000);
		clr_flg(Fsens, ~eRtouch);
		clr_flg(Fsens, ~eLtouch);
		ecrobot_sound_tone(110, 50, 60);
		dly_tsk(1000);
	}
}

// Event flag update routine
void
ESetTsk(VP_INT exinf)
{
	for (;;) {
		dly_tsk(10);
		if (ecrobot_get_touch_sensor(Rtouch)) {
			set_flg(Fsens, eRtouch);
		}
		if (ecrobot_get_touch_sensor(Ltouch)) {
			set_flg(Fsens, eLtouch);
		}
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
	;
}

// System Shutdown Routine
void
ecrobot_device_terminate(void)
{
	;
}
