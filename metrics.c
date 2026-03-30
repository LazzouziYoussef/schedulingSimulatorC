#include "metrics.h"

int TR(Proc *p) {
    return p->fin - p->da;
}

int TA(Proc *p) {
    return TR(p) - p->te;
}

float TRM(Result *r) {
    float s = 0;
    for (int i = 0; i < r->n; i++) s += TR(&r->procs[i]);
    return s / r->n;
}

float TAM(Result *r) {
    float s = 0;
    for (int i = 0; i < r->n; i++) s += TA(&r->procs[i]);
    return s / r->n;
}
