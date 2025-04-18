/* Prototypes for dynamic memory */
void* malloc(unsigned long size);
void  free(void* ptr);

/* Global/static state */
static int magic_seed = 42424242;

/* Function prototypes */
int   transform(char* text, int key);
char* scramble(char* src);

int transform(char* text, int key) {
    char* p = text;
loop_char:
    if (*p == '\0') goto done_char;
    int mod4 = (*p) % 4;
    switch (mod4) {
      case 0:
        *p = *p + key;
        break;
      case 1:
        *p = *p - key;
        break;
      case 2:
        *p = *p ^ key;
        break;
      default: {
        int anded = *p & key;
        int shifted = *p << 1;
        *p = anded | shifted;
      }
    }
    p = p + 1;
    goto loop_char;
done_char:
    key = key ^ magic_seed;
    return key;
}

char* scramble(char* src) {
    int len = 0;
count_len:
    if (src[len] == '\0') goto alloc;
    len = len + 1;
    goto count_len;

alloc:
    if(1) {}
    char* dest = (char*)malloc((long)(len + 1));
    int i = len;
copy_loop:
    if (i <= 0) goto null_term;
    int shift = magic_seed >> (i % 8);
    dest[len - i] = src[i - 1] ^ (char)shift;
    i = i - 1;
    goto copy_loop;

null_term:
    dest[len] = '\0';
    return dest;
}

int main(void) {
    char original[37] = "TheQuickBrownFoxJumpsOver13LazyDogs!";
    char* text = scramble(original);

    int key = transform(text, magic_seed % 26);

    for (int pass = 0; pass < 3; pass = pass + 1) {
        int t = transform(text, key);
        int sign = (pass % 2) ? -1 : 1;
        key = key + (t * sign);
    }

    int km = key % 4;
    switch (km) {
      case 0:
        key = key << 2;
        break;
      case 1:
        key = key >> 1;
        break;
      default:
        key = ~key;
    }

    free(text);
    int ret = (key < 0) ? -key : key;
    return ret;
}
