#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

#ifdef __i386
uint64_t rdtsc() {
   uint64_t x;
   __asm__ volatile ("rdtsc" : "=A" (x));
   return x;
}
#elif defined __amd64
uint64_t rdtsc() {
   uint64_t a, d;
   __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
   return (d<<32) | a;
}
#endif

#define NB_METAS 31

typedef struct {
  double * restrict x;
  double * restrict y;
  double * restrict z;
} elem_t;

void baseline (unsigned n, elem_t a, elem_t s) {
  unsigned i;
  double x=1.0, y=1.0, z=1.0;

#pragma omp parallel for reduction(+ : x,y,z) schedule(static)
  for(i=0; i < n*n; i++) {
    x += a.x[i];
    y += a.y[i];
    z += a.z[i];
  }

  *(s.x) = x;
  *(s.y) = y;
  *(s.z) = z;
}

static void init_array (int n, elem_t a) {
  int i;

  for (i=0; i<n*n; i++) {
    a.x[i] = (float) rand() / RAND_MAX;
    a.y[i] = (float) rand() / RAND_MAX;
    a.z[i] = (float) rand() / RAND_MAX;
  }
}


int main(int argc, char** argv) {

  /* check command line arguments */
  if (argc != 4) {
    fprintf (stderr, "Usage: %s <size> <nb warmup repets> <nb measure repets>\n", argv[0]);
    abort();
  }

  int i, m;
  double avgt = 0.0;

  /* get command line arguments */
  unsigned size = atoi (argv[1]); /* matrix size */
  unsigned repw = atoi (argv[2]); /* repetition number */
  unsigned repm = atoi (argv[3]); /* repetition number */

  for (m=0; m<NB_METAS; m++) {
    /* allocate arrays */
    elem_t a;
    elem_t b;

    a.x = malloc(size*size*sizeof(double));
    a.y = malloc(size*size*sizeof(double));
    a.z = malloc(size*size*sizeof(double));
    b.x = malloc(sizeof(double));
    b.y = malloc(sizeof(double));
    b.z = malloc(sizeof(double));

    /* init arrays */
    srand(0);
    init_array (size, a);

    /* warmup (repw repetitions in first meta, 1 repet in next metas) */
    if (m == 0) {
      for (i=0; i<repw; i++)
        baseline(size, a, b);
      printf("%lf %lf %lf\n", *(b.x), *(b.y), *(b.z));
    } else {
      baseline(size, a, b);
      printf("%lf %lf %lf\n", *(b.x), *(b.y), *(b.z));
    }

    /* measure repm repetitions */
    uint64_t t1 = rdtsc();
    for (i=0; i<repm; i++)
      baseline(size, a, b);
    uint64_t t2 = rdtsc();


    /* print output */
      printf("%lf %lf %lf\n", *(b.x), *(b.y), *(b.z));

    double cpi = (t2 - t1) / (float)(size * size * repm);

    /* print performance */
    fprintf (stderr, "%.2f cycles/iter\n",
        cpi);

    avgt += cpi;

    /* free arrays */
    free (a.x);
    free (a.y);
    free (a.z);
    
    free (b.x);
    free (b.y);
    free (b.z);
  }

  fprintf(stderr, "avg : %.2lf\n", avgt / NB_METAS);

  return EXIT_SUCCESS;
}
