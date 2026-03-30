#ifndef METRICS_H
#define METRICS_H

#include "process.h"

int   TR  (Proc *p);           /* temps de réponse    = fin - da         */
int   TA  (Proc *p);           /* temps d'attente     = TR - te          */
float TRM (Result *r);         /* temps de réponse moyen                 */
float TAM (Result *r);         /* temps d'attente moyen                  */

#endif
