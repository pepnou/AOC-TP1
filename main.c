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
  double x;
  double y;
  double z;
} elem_t;

elem_t baseline (unsigned n, elem_t a[n][n]) {
  unsigned i, j;
  elem_t s = {1.0, 1.0, 1.0};

  for(j=0; j<n; j++) {
    for (i=0; i<n; i++) {
      s.x += a[i][j].x;
      s.y += a[i][j].y;
      s.z += a[i][j].z;
    }
  }

  return s;
}

static void init_array (int n, elem_t a[n][n]) {
   int i, j;

   for (i=0; i<n; i++)
      for (j=0; j<n; j++) {
         a[i][j].x = (float) rand() / RAND_MAX;
         a[i][j].y = (float) rand() / RAND_MAX;
         a[i][j].z = (float) rand() / RAND_MAX;
      }
}


int main(int argc, char** argv) {

  /* check command line arguments */
  if (argc != 4) {
    fprintf (stderr, "Usage: %s <size> <nb warmup repets> <nb measure repets>\n", argv[0]);
    abort();
  }

  int i, m;

  /* get command line arguments */
  unsigned size = atoi (argv[1]); /* matrix size */
  unsigned repw = atoi (argv[2]); /* repetition number */
  unsigned repm = atoi (argv[3]); /* repetition number */

  for (m=0; m<NB_METAS; m++) {
    /* allocate arrays */
    elem_t (*a)[size] = malloc(size * size * sizeof(elem_t));
    elem_t b;

    /* init arrays */
    srand(0);
    init_array (size, a);

    /* warmup (repw repetitions in first meta, 1 repet in next metas) */
    if (m == 0) {
      for (i=0; i<repw; i++)
        b = baseline(size, a);
    } else {
      b = baseline(size, a);
    }

    /* measure repm repetitions */
    uint64_t t1 = rdtsc();
    for (i=0; i<repm; i++)
      b = baseline(size, a);
    uint64_t t2 = rdtsc();


    /* print output */
      printf("%lf %lf %lf\n", b.x, b.y, b.z);

    /* print performance */
    fprintf (stderr, "%ld cycles/iter\n",
        (t2 - t1) / (size * size));

    /* free arrays */
    free (a);
  }

  return EXIT_SUCCESS;
}
