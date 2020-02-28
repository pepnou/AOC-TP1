#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern __inline__ uint64_t rdtsc(void) {
  uint64_t x;
  __asm__ volatile ("rdtsc" : "=A" (x));
  return x;
}

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
}


int main(int argc, char** argv) {

  /* check command line arguments */
  if (argc != 4) {
    fprintf (stderr, "Usage: %s <size> <nb warmup repets> <nb measure repets>\n", argv[0]);
    abort();
  }

  int i, m;

  /* get command line arguments */
  int size = atoi (argv[1]); /* matrix size */
  int repw = atoi (argv[2]); /* repetition number */
  int repm = atoi (argv[3]); /* repetition number */

  for (m=0; m<NB_METAS; m++) {
    /* allocate arrays */
    float (*a)[size] = malloc (size * size * sizeof *a);
    float (*b)[size] = malloc (size * size * sizeof *b);
    float (*c)[size] = malloc (size * size * sizeof *c);

    /* init arrays */
    srand(0);
    init_array (size, a);
    init_array (size, b);

    /* warmup (repw repetitions in first meta, 1 repet in next metas) */
    if (m == 0) {
      for (i=0; i<repw; i++)
        sgemm (size, a, b, c);
    } else {
      sgemm (size, a, b, c);
    }

    /* measure repm repetitions */
    uint64_t t1 = rdtsc();
    for (i=0; i<repm; i++)
      sgemm (size, a, b, c);
    uint64_t t2 = rdtsc();

    /* print performance */
    printf ("%.2f cycles/FMA\n",
        (t2 - t1) / ((float) size * size * size * repm));

    /* print output */
    //if (m == 0) print_array (n, c);

    /* free arrays */
    free (a);
    free (b);
    free (c);


    return EXIT_SUCCESS;
  }
