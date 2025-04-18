/* Prototypes for dynamic memory */
void *malloc(unsigned long size);
void free(void *ptr);

/* Global/static state */
static unsigned long global_seed = 123456789UL;

/* Function prototypes */
double compute_pattern(double *data, int size, double factor);
long process(unsigned int n, double factor);

double compute_pattern(double *data, int size, double factor) {
  int idx = 0;
  double acc = 1.0;

pattern_loop:
  if (idx >= size)
    goto finish_pattern;

  double v = *(data + idx);
  int imod = (int)v % 5;
  switch (imod) {
  case 0:
    /* acc = acc * (v - factor); */
    acc = acc * (v - factor);
    break;
  case 1:
    /* acc = acc + (v / (factor + 1.0)); */
    acc = acc + (v / (factor + 1.0));
    break;
  default:
    /* acc = acc - (v * factor); */
    acc = acc - (v * factor);
  }

  /* mix in some bitwise via reinterpretation */
  {
    unsigned long *bits = (unsigned long *)&v;
    unsigned long shifted = global_seed >> (idx % 8);
    unsigned long bx = *bits ^ shifted;
    *bits = bx;
    v = *(double *)bits;
  }

  *(data + idx) = v;
  idx = idx + 1;
  goto pattern_loop;

finish_pattern:
  return acc;
}

long process(unsigned int n, double factor) {
  double *arr = (double *)malloc((long)n * sizeof(double));
  unsigned int i = 0;
  while (i < n) {
    unsigned long tmp = global_seed ^ i;
    arr[i] = (double)(tmp + (long)factor);
    i = i + 1;
  }

  double r = compute_pattern(arr, (int)n, factor);

  unsigned int k = 0;
sum_loop:
  if (k >= n)
    goto end_sum;
  r = r + *(arr + k);
  k = k + 1;
  goto sum_loop;
end_sum:
  free(arr);

  /* final scramble with bitwise */
  long out = (long)r ^ (long)(factor * 1000.0);
  /* out = out + ((out & 255) << 3); */
  out = out + ((out & 255) << 3);
  return out;
}

int main(void) {
  unsigned int count = 127;
  double scale = 3.141592653589793;
  long result = process(count, scale);

  /* do-while, no compounds */
do_again:
  result = result >> 2;
  long tmp1 = result * 3;
  result = result - tmp1;
  result = result / 5;
  if (result > 1000)
    goto do_again;

  /* return via conditional operator */
  long final = (result < 0) ? -result : result;
  return (int)(final % 65536);
}
