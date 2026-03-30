#include "sched.h"
#include <stdlib.h>

/* ── helpers ────────────────────────────────────────────────────────────────
 */

static void init_result(Result *r, Proc procs[], int n) {
  r->n = n;
  r->nb_slots = 0;
  r->nb_ctx = 0;
  for (int i = 0; i < n; i++) {
    r->procs[i] = procs[i];
    r->procs[i].reste = procs[i].te;
    r->procs[i].fin = -1;
  }
}

static void push_slot(Result *r, int ts, int te, int idx) {
  r->gantt[r->nb_slots++] = (Slot){ts, te, idx};
  r->nb_ctx++;
}

static int cmp_da(const void *a, const void *b) {
  return ((Proc *)a)->da - ((Proc *)b)->da;
}

/* ── FCFS ───────────────────────────────────────────────────────────────────
 */

void fcfs(Proc procs[], int n, Result *r) {
  init_result(r, procs, n);
  qsort(r->procs, n, sizeof(Proc), cmp_da);
  int t = 0;
  for (int i = 0; i < n; i++) {
    if (t < r->procs[i].da)
      t = r->procs[i].da;
    push_slot(r, t, t + r->procs[i].te, i);
    t += r->procs[i].te;
    r->procs[i].fin = t;
  }
}

/* ── SJF (non préemptif) ────────────────────────────────────────────────────
 */

void sjf(Proc procs[], int n, Result *r) {
  init_result(r, procs, n);
  qsort(r->procs, n, sizeof(Proc), cmp_da);
  int done[MAX_PROC] = {0};
  int t = 0, finished = 0;
  while (finished < n) {
    int best = -1;
    for (int i = 0; i < n; i++) {
      if (done[i] || r->procs[i].da > t)
        continue;
      if (best == -1 || r->procs[i].te < r->procs[best].te)
        best = i;
    }
    if (best == -1) {
      t++;
      continue;
    }
    push_slot(r, t, t + r->procs[best].te, best);
    t += r->procs[best].te;
    r->procs[best].fin = t;
    done[best] = 1;
    finished++;
  }
}

/* ── Priorité non préemptif (valeur la plus petite = plus prioritaire) ──────
 */

void priorite(Proc procs[], int n, Result *r) {
  init_result(r, procs, n);
  qsort(r->procs, n, sizeof(Proc), cmp_da);
  int done[MAX_PROC] = {0};
  int t = 0, finished = 0;
  while (finished < n) {
    int best = -1;
    for (int i = 0; i < n; i++) {
      if (done[i] || r->procs[i].da > t)
        continue;
      if (best == -1 || r->procs[i].prio < r->procs[best].prio)
        best = i;
    }
    if (best == -1) {
      t++;
      continue;
    }
    push_slot(r, t, t + r->procs[best].te, best);
    t += r->procs[best].te;
    r->procs[best].fin = t;
    done[best] = 1;
    finished++;
  }
}

/* ── SRT (Shortest Remaining Time, préemptif) ───────────────────────────────
 */

void srt(Proc procs[], int n, Result *r) {
  init_result(r, procs, n);
  int finished = 0, t = 0;
  int last = -1;
  while (finished < n) {
    int best = -1;
    for (int i = 0; i < n; i++) {
      if (r->procs[i].da > t || r->procs[i].fin != -1)
        continue;
      if (best == -1 || r->procs[i].reste < r->procs[best].reste)
        best = i;
    }
    if (best == -1) {
      t++;
      continue;
    }
    if (best != last && r->nb_slots > 0 &&
        r->gantt[r->nb_slots - 1].proc_idx == best) {
      r->gantt[r->nb_slots - 1].t_end++;
    } else {
      push_slot(r, t, t + 1, best);
    }
    r->procs[best].reste--;
    if (r->procs[best].reste == 0) {
      r->procs[best].fin = t + 1;
      finished++;
    }
    last = best;
    t++;
  }
}

/* ── Round Robin ────────────────────────────────────────────────────────────
 */

#define MAX_Q 2000
static int queue[MAX_Q];
static int qh, qt, qsz;
static void qinit() { qh = qt = qsz = 0; }
static void qpush(int v) {
  queue[qt] = v;
  qt = (qt + 1) % MAX_Q;
  qsz++;
}
static int qpop() {
  int v = queue[qh];
  qh = (qh + 1) % MAX_Q;
  qsz--;
  return v;
}
static int qempty() { return qsz == 0; }

void rr(Proc procs[], int n, int quantum, Result *r) {
  init_result(r, procs, n);
  qsort(r->procs, n, sizeof(Proc), cmp_da);
  qinit();
  int t = 0, na = 0, done = 0;
  while (na < n && r->procs[na].da == 0)
    qpush(na++);
  while (done < n) {
    if (qempty()) {
      t = r->procs[na].da;
      while (na < n && r->procs[na].da <= t)
        qpush(na++);
    }
    int cur = qpop();
    int run = (r->procs[cur].reste < quantum) ? r->procs[cur].reste : quantum;
    push_slot(r, t, t + run, cur);
    while (na < n && r->procs[na].da <= t + run)
      qpush(na++);
    r->procs[cur].reste -= run;
    t += run;
    if (r->procs[cur].reste == 0) {
      r->procs[cur].fin = t;
      done++;
    } else {
      qpush(cur);
    }
  }
}
