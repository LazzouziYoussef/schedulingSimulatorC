#include "print.h"
#include "process.h"
#include "sched.h"
#include <stdio.h>

static void saisie(Proc procs[], int *n) {
  printf("Nombre de processus : ");
  scanf("%d", n);
  for (int i = 0; i < *n; i++) {
    printf("\nProcessus %d\n", i + 1);
    printf("  Nom          : ");
    scanf("%7s", procs[i].name);
    printf("  Date arrivée : ");
    scanf("%d", &procs[i].da);
    printf("  Temps exec.  : ");
    scanf("%d", &procs[i].te);
    printf("  Priorité     : ");
    scanf("%d", &procs[i].prio);
    procs[i].reste = procs[i].te;
    procs[i].fin = -1;
  }
}

int main(void) {
  Proc procs[MAX_PROC];
  int n;
  Result r;

  printf("=== Ordonnanceur ===\n\n");
  saisie(procs, &n);

  printf("\nAlgorithme:\n");
  printf("  1. FCFS\n");
  printf("  2. SJF\n");
  printf("  3. Priorité (non préemptif)\n");
  printf("  4. SRT (préemptif)\n");
  printf("  5. Round Robin\n");
  printf("Choix : ");
  int choix;
  scanf("%d", &choix);

  switch (choix) {
  case 1:
    printf("\n--- FCFS ---\n");
    fcfs(procs, n, &r);
    break;
  case 2:
    printf("\n--- SJF ---\n");
    sjf(procs, n, &r);
    break;
  case 3:
    printf("\n--- Priorité ---\n");
    priorite(procs, n, &r);
    break;
  case 4:
    printf("\n--- SRT ---\n");
    srt(procs, n, &r);
    break;
  case 5: {
    int q;
    printf("Quantum : ");
    scanf("%d", &q);
    printf("\n--- Round Robin (q=%d) ---\n", q);
    rr(procs, n, q, &r);
    break;
  }
  default:
    printf("Choix invalide.\n");
    return 1;
  }

  print_gantt(&r);
  print_table(&r);
  return 0;
}
