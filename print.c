#include "print.h"
#include <stdio.h>
#include <string.h>

/* ── Table des résultats ────────────────────────────────────────────────────
 */

void print_table(Result *r) {
  printf("\n%-10s %-6s %-6s %-6s %-12s %-8s\n", "Processus", "DA", "TE", "Fin",
         "TR=Fin-DA", "TA=TR-TE");
  printf("%-60s\n",
         "------------------------------------------------------------");
  for (int i = 0; i < r->n; i++) {
    Proc *p = &r->procs[i];
    printf("%-10s %-6d %-6d %-6d %-12d %-8d\n", p->name, p->da, p->te, p->fin,
           TR(p), TA(p));
  }
  printf("\nTRM = %.2f\n", TRM(r));
  printf("TAM = %.2f\n", TAM(r));
  printf("Changements de contexte = %d\n", r->nb_ctx);
}

/* ── Gantt dans le terminal ─────────────────────────────────────────────────
 */

/*
 * Chaque unité de temps = 1 caractère '-'
 * Affichage:
 *   NOM| [espaces jusqu'au début] [tirets pour la durée]
 *
 * Exemple (quantum=1):
 *   A|------  -  -
 *   B| -  - ...
 */
void print_gantt(Result *r) {
  int t_max = 0;
  for (int s = 0; s < r->nb_slots; s++)
    if (r->gantt[s].t_end > t_max)
      t_max = r->gantt[s].t_end;

  /* t: header */
  printf("\nt:  ");
  for (int t = 0; t < t_max; t++)
    printf("%-3d", t);
  printf("\n");

  /* ligne pour chaque processus (- ou espace pour chaque unité de temps)*/
  for (int i = 0; i < r->n; i++) {
    char active[MAX_SLOTS] = {0};
    for (int s = 0; s < r->nb_slots; s++) {
      if (r->gantt[s].proc_idx != i)
        continue;
      for (int t = r->gantt[s].t_start; t < r->gantt[s].t_end; t++)
        active[t] = 1;
    }
    printf("%-2s: ", r->procs[i].name);
    for (int t = 0; t < t_max; t++)
      printf("%-3c", active[t] ? '-' : ' ');
    printf("\n");
  }
}