# Scheduling Library

Process scheduling simulator with five algorithms, metrics computation, and Gantt chart rendering.

## Project Structure

main.c — Interactive entry point; reads process list and algorithm choice via stdin.
process.h — Shared type definitions: `Proc`, `Slot`, `Result`.
sched.h / sched.c — Five scheduling algorithms: FCFS, SJF, priority, SRT, round-robin.
metrics.h / metrics.c — Metrics functions: TR, TA, TRM, TAM (response and wait times).
print.h / print.c — Output functions: tabular summary and Gantt chart display.
Makefile — Targets: `all`, `clean`, `re`.

## Build

Clone the repository and build:

```bash
git clone https://github.com/LazzouziYoussef/schedulingSimulatorC
cd schedulingSimulatorC
make
```

### Targets

```makefile
all     Build executable sched from main.c, sched.c, metrics.c, print.c
clean   Remove object files and executable
re      Clean then build
```

The `all` target expands to:

```c
gcc -Wall -I. -o sched main.o sched.o metrics.o print.o
```

Each `.o` is built from its `.c` via:

```c
gcc -Wall -I. -c <file>.c -o <file>.o
```

## Types (process.h)

### Proc

```c
typedef struct {
    char name[8];      /* process identifier */
    int  da;           /* arrival time (date d'arrivée) */
    int  te;           /* execution time needed (temps d'exécution) */
    int  prio;         /* priority: lower value = higher priority */
    int  reste;        /* remaining time (set and updated during scheduling) */
    int  fin;          /* completion time (set by algorithm, -1 until done) */
} Proc;
```

### Slot

```c
typedef struct {
    int t_start;       /* execution interval start */
    int t_end;         /* execution interval end (exclusive) */
    int proc_idx;      /* index into Result.procs; -1 = CPU idle */
} Slot;
```

Slot decouples the scheduling algorithm from the rendering pipeline. Algorithms write discrete execution intervals; the print layer reads them without re-executing the schedule. Each slot records a contiguous time window assigned to one process, enabling both Gantt rendering and metrics computation.

### Result

```c
typedef struct {
    Proc  procs[MAX_PROC];     /* input processes, enriched with fin and reste */
    int   n;                   /* process count */
    Slot  gantt[MAX_SLOTS];    /* scheduling output: array of execution intervals */
    int   nb_slots;            /* number of slots written */
    int   nb_ctx;              /* context switch count */
} Result;
```

Result carries all output from an algorithm call: the decorated process array (with `fin` and `reste` fields set), the gantt array (schedule as slots), and metadata. This structure serves as both the algorithm's scratchpad and the canonical representation passed to print and metrics functions.

## Algorithms (sched.c)

### FCFS (First-Come First-Served)

```c
void fcfs(Proc procs[], int n, Result *r)
```

Sort processes by arrival time. Iterate through them sequentially; when a process arrives before current time or is ready, start it immediately at max(current_time, arrival_time) and run it to completion. No preemption.

```c
if (t < r->procs[i].da) t = r->procs[i].da;
push_slot(r, t, t + r->procs[i].te, i);
t += r->procs[i].te;
r->procs[i].fin = t;
```

If the process has not yet arrived (t < da), advance time to its arrival. Record a slot from that time to time + execution_time. Update the completion time. This is the crux: CPU idles if needed, then runs the next arrived process to completion.

### SJF (Shortest Job First, non-preemptive)

```c
void sjf(Proc procs[], int n, Result *r)
```

At each decision point, scan all unscheduled, arrived processes and pick the one with minimum execution time. If no process is ready, advance time by 1 and retry. Non-preemptive: once a process starts, it runs to completion before the next one is chosen.

```c
for (int i = 0; i < n; i++) {
    if (done[i] || r->procs[i].da > t) continue;
    if (best == -1 || r->procs[i].te < r->procs[best].te) best = i;
}
```

Iterate all processes; skip those already finished or not yet arrived. Among the rest, pick the one with smallest `te`. This greedy choice minimizes average wait time for the current set of ready processes.

### Priorité (Priority, non-preemptive)

```c
void priorite(Proc procs[], int n, Result *r)
```

At each decision point, choose the arrived, unscheduled process with the smallest priority value. Lower `prio` means higher priority. Non-preemptive: the selected process runs to completion.

```c
for (int i = 0; i < n; i++) {
    if (done[i] || r->procs[i].da > t) continue;
    if (best == -1 || r->procs[i].prio < r->procs[best].prio) best = i;
}
```

Scan all processes and keep the one with the minimum `prio` among those arrived and not yet done. This implements static priority discipline without interruption.

### SRT (Shortest Remaining Time, preemptive)

```c
void srt(Proc procs[], int n, Result *r)
```

At each unit of time, select the arrived, incomplete process with the smallest remaining execution time. Run it for 1 unit. If the same process is chosen again, merge the new 1-unit slot with the previous one; otherwise start a fresh slot. This preemptive strategy minimizes average response time under variable job lengths.

```c
if (best != last && r->nb_slots > 0 &&
    r->gantt[r->nb_slots-1].proc_idx == best) {
    r->gantt[r->nb_slots-1].t_end++;
} else {
    push_slot(r, t, t+1, best);
}
```

If the chosen process is the same as the last one executed in the previous time unit, extend the end time of the last slot by 1; otherwise, create a new slot. This coalesces consecutive single-unit intervals into longer contiguous ones, producing a cleaner Gantt representation.

### RR (Round-Robin)

```c
void rr(Proc procs[], int n, int quantum, Result *r)
```

Maintain a FIFO queue of arrived processes. Each process runs for min(quantum, remaining_time), then returns to the back of the queue if not done. When the queue empties, advance time to the next arrival. This ensures fair CPU allocation and short response times for interactive workloads.

```c
int run = (r->procs[cur].reste < quantum) ? r->procs[cur].reste : quantum;
push_slot(r, t, t + run, cur);
while (na < n && r->procs[na].da <= t + run) qpush(na++);
r->procs[cur].reste -= run;
t += run;
if (r->procs[cur].reste == 0) {
    r->procs[cur].fin = t;
    done++;
} else {
    qpush(cur);
}
```

Dequeue the first ready process. Calculate its timeslice as the minimum of quantum and remaining time. Record a slot for that duration. Enqueue any newly arrived processes. Decrement the process's remaining time. If it has more work, re-enqueue it; otherwise mark it done. The quantum ensures no process monopolizes the CPU.

## Metrics (metrics.c)

### TR (Response Time)

```c
int TR(Proc *p) {
    return p->fin - p->da;
}
```

$$TR_i = date_{fin}(p_i) - date_{arrival}(p_i)$$

Total time from arrival to completion. Includes both execution and waiting.

### TA (Wait Time)

```c
int TA(Proc *p) {
    return TR(p) - p->te;
}
```

$$TA_i = TR_i - TE(p_i)$$

Time spent waiting for the CPU. Derived from response time minus actual execution time.

### TRM (Mean Response Time)

```c
float TRM(Result *r) {
    float s = 0;
    for (int i = 0; i < r->n; i++) s += TR(&r->procs[i]);
    return s / r->n;
}
```

$$TRM = \frac{1}{n} \sum_{i=1}^{n} TR_i$$

Average response time across all processes.

### TAM (Mean Wait Time)

```c
float TAM(Result *r) {
    float s = 0;
    for (int i = 0; i < r->n; i++) s += TA(&r->procs[i]);
    return s / r->n;
}
```

$$TAM = \frac{1}{n} \sum_{i=1}^{n} TA_i$$

Average wait time across all processes.

## Print (print.c)

### print_table

Outputs a tabular summary: process name, arrival time, execution time, completion time, and computed TR and TA for each process. Below the table are TRM, TAM, and the total number of context switches. This gives a quick quantitative view of algorithm performance.

### print_gantt

Uses a two-pass algorithm. The first pass builds a boolean array `active[t]` for each process, marking which time units it was scheduled. The second pass renders each process as a row with dashes (`-`) for execution, spaces for idle.

```c
for (int s = 0; s < r->nb_slots; s++) {
    if (r->gantt[s].proc_idx != i) continue;
    for (int t = r->gantt[s].t_start; t < r->gantt[s].t_end; t++)
        active[t] = 1;
}
printf("%-2s: ", r->procs[i].name);
for (int t = 0; t < t_max; t++)
    printf("%-3c", active[t] ? '-' : ' ');
printf("\n");
```

For each slot, if it belongs to the current process, mark the interval from t_start to t_end (exclusive) as active in the boolean array. Then, iterate the time axis and print either a dash or space using `%-3c` formatting (3 characters per unit time, left-aligned). Each process row aligns with the time scale header above.

## Execution Path

```
main
  saisie() — read process count and details from stdin
  algorithm choice from stdin
  
  ↓ (switch on choice)
  
  fcfs |  sjf  |  priorite  |  srt  |  rr
    │       │       │        │       │
    └───┬───┴───┬───┴───┬────┴───┬───┘
        │       │       │        │
        └───────┴───────┴────────┴─→  Result filled:
                                         • procs[i].fin  written
                                         • gantt[]        written
                                         • nb_slots       written
                                         • nb_ctx         written
                                         
        ↓ (Result structure populated)
        
        ├─→ print_gantt(Result)
        │     1. Compute t_max from gantt slots
        │     2. Print header: t:  0  1  2  3  ...
        │     3. For each process i:
        │        ├─ Build active[t] array from gantt slots
        │        ├─ Print "NAME: " row with %-3c columns
        │
        └─→ print_table(Result)
              1. Print header row
              2. For each process:
                 ├─ Call TR(p), TA(p) 
                 └─ Print row with metrics
              3. Call TRM(Result), TAM(Result)
              4. Print mean metrics and nb_ctx
```

The Result structure is the central artifact: algorithms write to it, and print functions read from it. Slots represent scheduled intervals; fin and reste track completion and progress. Metrics functions compute derived values directly from the Proc array inside Result.

## Example Output

Round-robin with quantum=2 on three processes (A arrives at 0 with TE 5, B at 2 with TE 3, C at 4 with TE 2):

```
t:  0  1  2  3  4  5  6  7  8  9  
A : -  -        -  -           -  
B :       -  -              -     
C :                   -  -        

Processus  DA     TE     Fin    TR=Fin-DA    TA=TR-TE
------------------------------------------------------------
A          0      5      10     10           5       
B          2      3      9      7            4       
C          4      2      8      4            2       

TRM = 7.00
TAM = 3.67
Changements de contexte = 6
```


