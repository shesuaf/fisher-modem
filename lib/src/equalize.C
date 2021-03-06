/* Modem for MIPS   AJF	  October 1996
   Equalizer routines */

#include <stdio.h>
#include <string.h>

#include "private.h"
#include "complex.h"
#include "equalize.h"

void equalizer::reset() {
  for (int i = 0; i < 2 * EQ_NP + 1; i++)
    coeffs[i] = 0.0;
  coeffs[EQ_NP] = 1.0;
  for (int i = 0; i < EQ_SIZE; i++)
    in[i] = 0.0;
  next = 0;
}

void equalizer::insert(complex z) { /* circular buffer */
  in[next] = z;
  if (++next >= EQ_SIZE)
    next = 0;
}

complex equalizer::get() { /* get equalized value */
  complex z = 0.0;
  int ncs = 2 * EQ_NP + 1;
  for (int i = 0; i < ncs; i++) {
    int p = (next + i) & (EQ_SIZE - 1);
    z += coeffs[i] * in[p];
  }
  return z;
}

void equalizer::upd(complex eps, int n) {
  complex deps = (delta / (2 * n + 1)) * eps;
  for (int i = EQ_NP - n; i <= EQ_NP + n; i++) {
    int p = (next + i) & (EQ_SIZE - 1);
    coeffs[i] += deps * cconj(in[p]);
  }
}

int equalizer::getdt() {
  int k1 = 0, k2 = 2 * EQ_NP;
  float p1 = 0.0, p2 = 0.0; /* total power to left of k1, right of k2 */
  while (k1 < k2) {
    if (p1 > p2)
      p2 += power(coeffs[k2--]);
    else
      p1 += power(coeffs[k1++]);
  }
  return k1 - EQ_NP; /* return timing (sample point) error, in units of half a
                        symbol */
}

void equalizer::shift(int dt) {
  int ncs = 2 * EQ_NP + 1;
  if (dt > 0) {
    memmove(&coeffs[dt], &coeffs[0], (ncs - dt) * sizeof(complex));
    memset(&coeffs[0], 0, dt * sizeof(complex));
  }
  if (dt < 0) {
    dt = -dt;
    memmove(&coeffs[0], &coeffs[dt], (ncs - dt) * sizeof(complex));
    memset(&coeffs[ncs - dt], 0, dt * sizeof(complex));
  }
}

void equalizer::print(char* fn) {
  FILE* fi = fopen(fn, "w");
  if (fi != NULL) {
    fprintf(fi, ".sp 0.5i\n.G1 8i\n");
    fprintf(fi, "new solid\n");
    for (int j = -EQ_NP; j <= +EQ_NP; j++)
      fprintf(fi, "%d %g\n", j, coeffs[EQ_NP + j].re);
    fprintf(fi, "new dashed\n");
    for (int j = -EQ_NP; j <= +EQ_NP; j++)
      fprintf(fi, "%d %g\n", j, coeffs[EQ_NP + j].im);
    fprintf(fi, "new dotted\n");
    for (int j = -EQ_NP; j <= +EQ_NP; j++)
      fprintf(fi, "%d %g\n", j, power(coeffs[EQ_NP + j]));
    fprintf(fi, ".G2\n.bp\n");
    for (int j = -EQ_NP; j <= +EQ_NP; j++)
      fprintf(fi, "{ %10.6f, %10.6f },\n", coeffs[EQ_NP + j].re,
              coeffs[EQ_NP + j].im);
    fclose(fi);
  } else
    fprintf(stderr, "can't create %s\n", fn);
}
