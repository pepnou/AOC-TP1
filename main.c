#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
  double *x;
  double *y;
  double *z;
} elem_t;

void baseline (unsigned n, elem_t a, elem_t s) {
  unsigned i;
  double x0=0.0, x1=0.0, x2=0.0, x3=0.0;
  double y0=0.0, y1=0.0, y2=0.0, y3=0.0;
  double z0=0.0, z1=0.0, z2=0.0, z3=0.0;

  for(i=0; i <= n*n - 4; i += 4) {
    x0 += a.x[i];
    x1 += a.x[i+1];
    x2 += a.x[i+2];
    x3 += a.x[i+3];

    y0 += a.y[i];
    y1 += a.y[i+1];
    y2 += a.y[i+2];
    y3 += a.y[i+3];

    z0 += a.z[i];
    z1 += a.z[i+1];
    z2 += a.z[i+2];
    z3 += a.z[i+3];
  }

  for(; i < n*n; i++) {
    x0 += a.x[i];
    y0 += a.y[i];
    z0 += a.z[i];
  }

  *(s.x) = x0+x1+x2+x3+1.0;
  *(s.y) = y0+y1+y2+y3+1.0;
  *(s.z) = z0+z1+z2+z3+1.0;
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

    /* print performance */
    fprintf (stderr, "%.2f cycles/iter\n",
        (t2 - t1) / (float)(size * size * repm));

    avgt += (t2 - t1);

    /* free arrays */
    free (a.x);
    free (a.y);
    free (a.z);
    free (b.x);
    free (b.y);
    free (b.z);
  }

  fprintf(stderr, "avg : %.2lf\n", avgt / (size * size * repm * NB_METAS));

  return EXIT_SUCCESS;
}
