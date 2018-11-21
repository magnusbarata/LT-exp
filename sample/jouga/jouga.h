/*
 * jouga.h
 *	ロボットに関する定数などを保持
 */

#ifndef JOUGA_H
#define JOUGA_H

#ifndef _MACRO_ONLY
#include "ecrobot_interface.h"	// ヘッダファイル中にガードあり

extern int lval, cval, llow, lhigh, clow, chigh;
int get_light_sensor(int);
#endif

// 一番最初のプログラム変更はおそらくこれらの値の変更
#define	LOWVAL (470)
#define	HIGHVAL (640)
#define LOWPOWER (50)
#define HIGHPOWER (100)

#ifndef _MACRO_ONLY
// ポートに関する定数値を列挙型として登録
typedef enum {
	Rmotor = NXT_PORT_B,
	Lmotor = NXT_PORT_C,
	Light  = NXT_PORT_S3,
	Color  = NXT_PORT_S4,
} DeviceConstants;
#endif

// モータの向きが変わったときにプログラムを大幅変更しなくて済むように
#ifdef REVERSE
# define motor_set_speed(M, P, F)	nxt_motor_set_speed((M), (-(P)), (F))
#else
# define motor_set_speed(M, P, F)	nxt_motor_set_speed((M), (P), (F))
#endif

#endif
