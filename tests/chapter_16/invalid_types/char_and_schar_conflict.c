/* char and signed char are different types; you can't use them
 * interchangeably when redeclaring the same identifier */
char c = 10;

int main(void) {
  // this conflicts with previous definition of char
  extern unsigned char c;
  return c;
}