#ifndef SCHED_H
#define SCHED_H

#include "process.h"

/* Tous les algorithmes prennent un tableau de Proc (non trié) et remplissent un Result */

void fcfs     (Proc procs[], int n, Result *r);
void sjf      (Proc procs[], int n, Result *r);
void priorite (Proc procs[], int n, Result *r);  /* non préemptif, plus petit = plus prioritaire */
void srt      (Proc procs[], int n, Result *r);  /* Shortest Remaining Time, préemptif           */
void rr       (Proc procs[], int n, int quantum, Result *r);

#endif
