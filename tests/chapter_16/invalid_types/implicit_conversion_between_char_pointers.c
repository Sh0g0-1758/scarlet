/* You can't implicitly convert a char * to a signed char * because they're
 * different types. */
int main(void) {
  char *c = 0;
  unsigned char *s = c;
  return (int)s;
}