/*
 * jouga_cfg.h
 *	jouga.cfgの中で用いる関数群の宣言
 *	定義されているのはjouga.c
 */
#ifndef JOUGA_CFG_H
#define JOUGA_CFG_H

#include <t_services.h>
#include "jouga.h"

#ifndef _MACRO_ONLY

/* Tasks */
extern void SensTsk(VP_INT exinf);
extern void NbtnTsk(VP_INT exinf);
extern void QuitTsk(VP_INT exinf);
extern void InitTsk(VP_INT exinf);
extern void MainTsk(VP_INT exinf);
extern void MoveTsk(VP_INT exinf);
extern void MotrTsk(VP_INT exinf);
extern void TimrTsk(VP_INT exinf);
extern void MuscTsk(VP_INT exinf);
extern void DispTsk(VP_INT exinf);
extern void ColsTsk(VP_INT exinf);

/* CyclicTimers */
extern void MoveCyc(VP_INT exinf);
extern void DispCyc(VP_INT exinf);

#endif

#endif
