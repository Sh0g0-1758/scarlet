/*
test3.s: Assembler messages:
test3.s:22: Error: symbol `.Llabel2' is already defined
test3.s:23: Error: symbol `.Llabel1' is already defined
[ERROR]: Failed to generate the executable
*/

int main(void) {
  int count = 0;
label2:
  count = count + 1;
  if (count > 10) {
    goto label1;
  } else {
    goto label2;
  }
label1:
  if (count > 20) {
    count = 0;
    goto label2;
  } else {
    goto label1;
  }
}
