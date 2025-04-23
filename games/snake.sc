char clear_screen_str[8] = { 27, '[', '2', 'J', 27, '[', 'H', '\0' };
char reset_str[5]        = { 27, '[', '0', 'm', '\0' };
char bold_str[5]         = { 27, '[', '1', 'm', '\0' };
char blue_str[8]         = { 27, '[', '1', ';', '3', '4', 'm', '\0' };
char yellow_str[8]       = { 27, '[', '1', ';', '3', '3', 'm', '\0' };
char red_str[8]          = { 27, '[', '1', ';', '3', '1', 'm', '\0' };
char white_str[8]        = { 27, '[', '1', ';', '3', '7', 'm', '\0' };
char green_str[8] = { 27, '[', '1', ';', '3', '2', 'm', '\0' };

int putchar(int c);
int getchar(void);
void *malloc(unsigned int size);
void free(void *ptr);

int WIDTH = 40;
int HEIGHT = 20;
int MAX_LEN = 100;

int snake_x[100];
int snake_y[100];
int snake_len = 5;
int snake_dir = 3;

int food_x = 10;
int food_y = 5;
int score = 0;
int over = 0;

void delay(int ms) {
  int i;
  int j;
  for (i = 0; i < ms; i++) {
    for (j = 0; j < 10000; j++) {
    }
  }
}

void print(char *s) {
  while (*s)
    putchar(*s++);
}

void clear_screen(void) {
  int i;
  for (i = 0; i < 50; i++) {
    putchar('\n');
  }
}

void print_num(int n) {
  int rev = 0;
  if (n == 0) {
    putchar('0');
    return;
  }
  while (n > 0) {
    rev = rev * 10 + (n % 10);
    n /= 10;
  }
  while (rev > 0) {
    putchar((rev % 10) + '0');
    rev /= 10;
  }
}

unsigned int rand_seed = 123456789;
int randmod(int m) {
  rand_seed = rand_seed * 1103515245U + 12345U;
  return ((rand_seed >> 16) & 32767) % m;
}

void place_food(void) {
  int i;
  int clash;
  do {
    food_x = 1 + randmod(WIDTH - 2);
    food_y = 1 + randmod(HEIGHT - 2);
    clash = 0;
    for (i = 0; i < snake_len; i++) {
      if (snake_x[i] == food_x && snake_y[i] == food_y) {
        clash = 1;
        break;
      }
    }
  } while (clash);
}
void draw(void) {
  int x;int y;int i;int is_body;

  print(clear_screen_str);  

  for (y = 0; y < HEIGHT; y++) {
    for (x = 0; x < WIDTH; x++) {
      if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1) {
        print(green_str);  
        putchar('#');
      } else if (x == snake_x[0] && y == snake_y[0]) {
        print(yellow_str); 
        putchar('O');
      } else if (x == food_x && y == food_y) {
        print(red_str);  
        putchar('*');
      } else {
        is_body = 0;
        for (i = 1; i < snake_len; i++) {
          if (snake_x[i] == x && snake_y[i] == y) {
            print(blue_str);  
            putchar('o');
            is_body = 1;
            break;
          }
        }
        if (!is_body) {
          print(reset_str);
          putchar(' ');
        }
      }
    }
    putchar('\n');
  }

  print(bold_str);
  print(white_str);
  print("Score: ");
  print_num(score);
  print(reset_str);
  putchar('\n');
}


void move(void) {
  int i;
  for (i = snake_len - 1; i > 0; i--) {
    snake_x[i] = snake_x[i - 1];
    snake_y[i] = snake_y[i - 1];
  }

  if (snake_dir == 0)
    snake_y[0]--;
  if (snake_dir == 1)
    snake_y[0]++;
  if (snake_dir == 2)
    snake_x[0]--;
  if (snake_dir == 3)
    snake_x[0]++;

  if (snake_x[0] <= 0 || snake_x[0] >= WIDTH - 1 || snake_y[0] <= 0 ||
      snake_y[0] >= HEIGHT - 1) {
    over = 1;
    return;
  }

  for (i = 1; i < snake_len; i++) {
    if (snake_x[i] == snake_x[0] && snake_y[i] == snake_y[0]) {
      over = 1;
      return;
    }
  }

  if (snake_x[0] == food_x && snake_y[0] == food_y) {
    if (snake_len < MAX_LEN) {
      snake_x[snake_len] = snake_x[snake_len - 1];
      snake_y[snake_len] = snake_y[snake_len - 1];
      snake_len++;
    }
    score++;
    place_food();
  }
}

void input(void) {
  int c;
  print("Move (WASD or Q to quit): ");
  c = getchar();
  if (c == 'w' && snake_dir != 1)
    snake_dir = 0;
  if (c == 's' && snake_dir != 0)
    snake_dir = 1;
  if (c == 'a' && snake_dir != 3)
    snake_dir = 2;
  if (c == 'd' && snake_dir != 2)
    snake_dir = 3;
  if (c == 'q')
    over = 1;
}

void gameover(void) {
  print(red_str);
  print(bold_str);
  print("GAME OVER\n");
  print(white_str);
  print("Final Score: ");
  print_num(score);
  print("\n");
  print(reset_str);
  print("Press Enter to exit...\n");
  getchar();
}

int main(void) {
  int i;
  for (i = 0; i < snake_len; i++) {
    snake_x[i] = (WIDTH / 2) - i;
    snake_y[i] = HEIGHT / 2;
  }
  place_food();

  while (!over) {
    clear_screen();
    draw();
    input();
    move();
    delay(50);
  }

  clear_screen();
  gameover();
  return 0;
}
