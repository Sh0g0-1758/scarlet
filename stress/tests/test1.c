int main(void) {
  int a;
  int b;
  int c;
  int d;
  int e;
  int f;
  int x;
  int y;
  int z;
  int i;
  int j;
  int k;
  int l;
  int m;
  int n;
  int o;
  int p;

start:
  if ((a = 25) > 0) {
    if ((b = a * 3) < 100) {
      c = 42;
      goto compute_d;
    } else {
      c = -42;
      goto compute_x;
    }
  } else {
    b = -75;
    goto end;
  }

compute_d:
  if ((d = b / 2) != 0) {
    e = d + 5;
    if (e > 50)
      goto compute_f;
    else
      goto compute_x;
  }
  goto end;

compute_f:
  f = e * 2;
  if (f < 100)
    goto compute_x;
  else {
    f = f / 2;
    goto compute_x;
  }

compute_x:
  x = (c + d) * 2;
  if (x > 100) {
    y = x / 2;
    goto compute_z;
  } else {
    y = x * 2;
    if (y < 50)
      goto end;
    goto compute_z;
  }

compute_z:
  z = x + y;
  if (z > 200) {
    i = z / 2;
    j = i - 50;
    goto compute_kl;
  } else {
    i = z * 2;
    j = i + 25;
    if (j > 300)
      goto end;
    goto compute_kl;
  }

compute_kl:
  k = i + j;
  l = k - 100;
  if (k > l) {
    m = k / 2;
    n = l * 2;
    goto compute_op;
  } else {
    goto end;
  }

compute_op:
  o = m + n;
  p = o - 50;
  goto final;

end:
  return -1;

final:
  return (!(~a & 255) && (((x << 2) | (y >> 1)) ^ (z + -b)) > (c * d / e % f) ||
          (i <= j) != (k >= l)) &&
         ((m < n) == (o > p));
}
