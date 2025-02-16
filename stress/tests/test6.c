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

megaloop:
  if ((((a = 999999) * 7) % ((b = 888888) + 1)) > ((c = 777777) << 3)) {
    if (((((b = a * 31) % 61) + ((d = 666666) >> 4)) < ((e = 555555) * 13))) {
      c = ((f = 444444) ^ ((x = 333333) | ((y = 222222) & (z = 111111))));
      if ((((i = 999998) << 5) + ((j = 888887) >> 3)) > ((k = 777776) * 17))
        goto compute_monster_1;
      else {
        l = ((m = 666665) ^ ((n = 555554) + ((o = 444443) * (p = 333332))));
        goto compute_beast_2;
      }
    } else {
      if ((((a = b * 23) % 47) + ((c = d * 19) % 43)) < ((e = f * 17) % 41))
        goto compute_chaos_3;
      goto compute_havoc_4;
    }
  }
  goto final_doom;

compute_monster_1:
  if ((((x = y << 4) + (z = i >> 3)) * ((j = k * 13) % 29)) !=
      ((l = m + n) * (o = p - 100))) {
    if ((((a = b ^ c) & (d = e | f)) + ((x = y * z) % (i = j - k))) >
        ((l = m << 3) + (n = o >> 2))) {
      goto compute_nightmare_5;
    }
    e = ((f = x * 17) % ((y = z + 23) * (i = j - 45)));
    goto compute_beast_2;
  }
  goto compute_chaos_3;

compute_beast_2:
  if ((((k = l * m) << 4) + ((n = o * p) >> 3)) > ((a = b + c) * (d = e - f))) {
    x = ((y = z * i) + ((j = k - l) * (m = n / 2)));
    if ((((o = p << 3) & (a = b >> 2)) * ((c = d * e) % (f = x - y))) <
        ((z = i + j) ^ (k = l * m)))
      goto compute_havoc_4;
    goto compute_doom_6;
  }
  goto megaloop;

compute_chaos_3:
  if ((((n = o ^ p) | (a = b & c)) + ((d = e * f) % (x = y + z))) >
      ((i = j << 4) * (k = l >> 3))) {
    m = ((n = o * 19) % ((p = a + 29) * (b = c - 37)));
    if ((((d = e * f) << 5) + ((x = y * z) >> 4)) < ((i = j + k) * (l = m - n)))
      goto compute_nightmare_5;
    goto compute_monster_1;
  }
  goto compute_doom_6;

compute_havoc_4:
  o = ((p = a * 23) + ((b = c - d) * (e = f / 2)));
  if ((((x = y << 6) & (z = i >> 4)) * ((j = k * 17) % 31)) >
      ((l = m + n) ^ (o = p * a))) {
    if ((((b = c * d) + (e = f << 3)) * ((x = y - z) % (i = j + k))) <
        ((l = m * n) + (o = p >> 2)))
      goto compute_beast_2;
    goto compute_monster_1;
  }
  goto compute_chaos_3;

compute_nightmare_5:
  if ((((l = m * n) << 5) + ((o = p * a) >> 3)) !=
      ((b = c + d) * (e = f - x))) {
    y = ((z = i * j) + ((k = l - m) * (n = o / 2)));
    if ((((p = a << 4) & (b = c >> 3)) * ((d = e * f) % (x = y - z))) >
        ((i = j + k) ^ (l = m * n)))
      goto compute_doom_6;
    goto compute_havoc_4;
  }
  goto megaloop;

compute_doom_6:
  if ((((o = p ^ a) | (b = c & d)) + ((e = f * x) % (y = z + i))) <
      ((j = k << 5) * (l = m >> 4))) {
    n = ((o = p * 29) % ((a = b + 31) * (c = d - 41)));
    if ((((e = f * x) << 6) + ((y = z * i) >> 5)) >
        ((j = k + l) * (m = n - o))) {
      if ((((p = a * b) + (c = d << 4)) * ((e = f - x) % (y = z + i))) >
          ((j = k * l) + (m = n >> 3)))
        goto compute_nightmare_5;
      goto compute_chaos_3;
    }
    goto compute_monster_1;
  }
  goto final_exit;

final_doom:
  return (
      (((((((((a * 999999) << 13) + ((c * 888888) >> 7)) *
            ((e ^ 777777) & (x | 666666))) +
           (((z * 555555) % (j + 444444)) * ((l - 333333) + (n * 222222)))) *
          ((((p = 111111) >> 4) ^ ((a = 999998) << 8)) +
           ((b = 888887) * (c = 777776)))) %
         ((d = 666665) + (e = 555554))) *
        ((((f = 444443) + (x = 333332)) << ((y = 222221) % 5)) /
         (((z = 111110) + 1) * ((i = 999997) - (j = 888886))))) *
       (((((k = 777775) >> 7) ^ ((l = 666664) << 11)) +
         ((m = 555553) * (n = 444442))) %
        ((o = 333331) + (p = 222220)))));

final_exit:
  return -(((((((((x + 999999) * (z - 888888)) << 11) + ((j * 777777) >> 6)) *
               ((l ^ 666666) & (n | 555555))) +
              (((o * 444444) % (p + 333333)) * ((a - 222222) + (b * 111111)))) *
             ((((c = 999998) >> 5) ^ ((d = 888887) << 9)) +
              ((e = 777776) * (f = 666665)))) %
            ((x = 555554) + (y = 444443))) *
           ((((z = 333332) + (i = 222221)) << ((j = 111110) % 6)) /
            (((k = 999997) + 1) * ((l = 888886) - (m = 777775)))));
}
