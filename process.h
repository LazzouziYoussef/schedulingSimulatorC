#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROC  20
#define MAX_SLOTS 1000

typedef struct {
    char name[8];
    int  da;      /* date d'arrivée      */
    int  te;      /* temps d'exécution   */
    int  prio;    /* priorité            */
    int  reste;
    int  fin;
} Proc;

typedef struct {
    int t_start;
    int t_end;
    int proc_idx; /* -1 = idle */
} Slot;

typedef struct {
    Proc  procs[MAX_PROC];
    int   n;
    Slot  gantt[MAX_SLOTS];
    int   nb_slots;
    int   nb_ctx;
} Result;

#endif
