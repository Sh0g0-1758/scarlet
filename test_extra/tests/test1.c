int printf(char *format, ...);

int main(void) {
    int a = 42;
    double pi = 3.1415926535;
    char ch = 'A';
    char *str = "Hello, World!";
    void *ptr = (void *)&a;
    return printf("Integer with padding and sign: %+08d\nHexadecimal (uppercase): %#X\nOctal with prefix: %#o\nFloating point (default): %f\nFloating point (scientific): %e\nFloating point (precision 3): %.3f\nCharacter: %c\nString (15 wide, left-aligned): %-15s<END>\nPointer address: %p\nPercent sign: %%\nEscape sequences: newline\\n, tab\\t \tDone.\nCrazy mix: |%#10x|%-10d|%+10.2f|%c|%-20.5s|\nEmbedded quotes: printf(\"She said, \\\"Hello!\\\"\");\n", a, a, a, pi, pi, pi, ch, str, ptr, a, a, pi, ch, str);
}
