extern int putchar(int a);
extern int puts(char *a);
extern int getchar(void);
extern void srand(unsigned int a);
extern int rand(void);
extern long time(long *a);

char clear_screen_str[8] = { 27, '[', '2', 'J', 27, '[', 'H', '\0' };
char reset_str[5]        = { 27, '[', '0', 'm', '\0' };
char bold_str[5]         = { 27, '[', '1', 'm', '\0' };
char blue_str[8]         = { 27, '[', '1', ';', '3', '4', 'm', '\0' };
char yellow_str[8]       = { 27, '[', '1', ';', '3', '3', 'm', '\0' };
char red_str[8]          = { 27, '[', '1', ';', '3', '1', 'm', '\0' };
char white_str[8]        = { 27, '[', '1', ';', '3', '7', 'm', '\0' };

#define MAZE_WIDTH  28
#define MAZE_HEIGHT 11
#define NUM_GHOSTS  3

char maze[11][29] = {
    "############################",
    "#............##............#",
    "#.####.#####.##.#####.####.#",
    "#..........................#",
    "#.####.##.########.##.####.#",
    "#......##....##....##......#",
    "#.####.#####.##.#####.####.#",
    "#..........................#",
    "#.####.#####.##.#####.####.#",
    "#............##............#",
    "############################"
};

void my_print(char *s)
{
    while (*s)
    {
        putchar(*s);
        s++;
    }
}

void print_int(int num)
{
    char buf[12];
    int i;
    int neg;
    
    i = 10;
    neg = 0;
    buf[11] = '\0';
    
    if (num < 0)
    {
        neg = 1;
        num = -num;
    }
    
    if (num == 0)
    {
        buf[i] = '0';
        i--;
    }
    
    while (num > 0 && i >= 0)
    {
        buf[i] = '0' + (num % 10);
        num = num / 10;
        i--;
    }
    
    if (neg && i >= 0)
    {
        buf[i] = '-';
        i--;
    }
    
    my_print(&buf[i + 1]);
}

void clear_screen(void)
{
    my_print(clear_screen_str);
}

void clear_input_buffer(void)
{
    int c;
    c = 0;
    while ((c = getchar()) != '\n' && c != -1)
    {
    }
}

int count_pellets(void)
{
    int count;
    int y;
    int x;
    
    count = 0;
    
    for (y = 0; y < MAZE_HEIGHT; y++)
    {
        for (x = 0; x < MAZE_WIDTH; x++)
        {
            if (maze[y][x] == '.')
            {
                count++;
            }
        }
    }
    
    return count;
}

void draw_maze(int player_x, int player_y, int* ghost_x, int* ghost_y)
{
    int y;
    int x;
    int i;
    
    clear_screen();
    my_print(bold_str);
    my_print("Retro Pac-Man Style Game\n");
    my_print(reset_str);
    my_print("Pellets remaining: ");
    print_int(count_pellets());
    my_print("\n\n");
    
    for (y = 0; y < MAZE_HEIGHT; y++)
    {
        for (x = 0; x < MAZE_WIDTH; x++)
        {
            int printed;
            printed = 0;
            
            if (player_x == x && player_y == y)
            {
                my_print(yellow_str);
                putchar('P');
                my_print(reset_str);
                printed = 1;
            }
            else
            {
                for (i = 0; i < NUM_GHOSTS; i++)
                {
                    if (ghost_x[i] == x && ghost_y[i] == y)
                    {
                        my_print(red_str);
                        putchar('G');
                        my_print(reset_str);
                        printed = 1;
                        break;
                    }
                }
            }
            
            if (!printed)
            {
                char cell;
                cell = maze[y][x];
                if (cell == '#')
                {
                    my_print(blue_str);
                    putchar(cell);
                    my_print(reset_str);
                }
                else if (cell == '.')
                {
                    my_print(white_str);
                    putchar(cell);
                    my_print(reset_str);
                }
                else
                {
                    putchar(cell);
                }
            }
        }
        putchar('\n');
    }
    my_print("\nUse WASD keys to move (q to quit): ");
}

int is_walkable(int nx, int ny)
{
    if (nx < 0 || nx >= MAZE_WIDTH || ny < 0 || ny >= MAZE_HEIGHT)
    {
        return 0;
    }
    
    if (maze[ny][nx] == '#')
    {
        return 0;
    }
    
    return 1;
}

int main(void)
{
    int player_x;
    int player_y;
    int ghost_x[NUM_GHOSTS];
    int ghost_y[NUM_GHOSTS];
    char input;
    int newX;
    int newY;
    int i;
    
    srand((unsigned int)time((long *)0));
    
    player_x = 1;
    player_y = 1;
    
    ghost_x[0] = 14;
    ghost_y[0] = 1;
    ghost_x[1] = 13;
    ghost_y[1] = 5;
    ghost_x[2] = 14;
    ghost_y[2] = 9;
    
    while (1)
    {
        draw_maze(player_x, player_y, ghost_x, ghost_y);
        
        input = getchar();
        clear_input_buffer();
        
        if (input == 'q' || input == 'Q')
        {
            my_print("\nYou quit the game. Goodbye!\n");
            break;
        }
        
        newX = player_x;
        newY = player_y;
        
        if (input == 'w' || input == 'W')
        {
            newY--;
        }
        else if (input == 's' || input == 'S')
        {
            newY++;
        }
        else if (input == 'a' || input == 'A')
        {
            newX--;
        }
        else if (input == 'd' || input == 'D')
        {
            newX++;
        }
        else
        {
            continue;
        }
        
        if (is_walkable(newX, newY))
        {
            player_x = newX;
            player_y = newY;
            if (maze[player_y][player_x] == '.')
            {
                maze[player_y][player_x] = ' ';
            }
        }
        
        if (count_pellets() == 0)
        {
            draw_maze(player_x, player_y, ghost_x, ghost_y);
            my_print("\n");
            my_print(bold_str);
            my_print("Congratulations! You have eaten all the pellets and won!\n");
            my_print(reset_str);
            break;
        }
        
        for (i = 0; i < NUM_GHOSTS; i++)
        {
            int cur_x;
            int cur_y;
            int best_x;
            int best_y;
            int best_dist;
            int cand_x;
            int cand_y;
            int dist;
            int dx;
            int dy;
            
            cur_x = ghost_x[i];
            cur_y = ghost_y[i];
            best_x = cur_x;
            best_y = cur_y;
            best_dist = 100000;
            
            cand_x = cur_x;
            cand_y = cur_y - 1;
            if (is_walkable(cand_x, cand_y))
            {
                dx = (cand_x > player_x) ? (cand_x - player_x) : (player_x - cand_x);
                dy = (cand_y > player_y) ? (cand_y - player_y) : (player_y - cand_y);
                dist = dx + dy;
                if (dist < best_dist)
                {
                    best_dist = dist;
                    best_x = cand_x;
                    best_y = cand_y;
                }
            }
            
            cand_x = cur_x + 1;
            cand_y = cur_y;
            if (is_walkable(cand_x, cand_y))
            {
                dx = (cand_x > player_x) ? (cand_x - player_x) : (player_x - cand_x);
                dy = (cand_y > player_y) ? (cand_y - player_y) : (player_y - cand_y);
                dist = dx + dy;
                if (dist < best_dist)
                {
                    best_dist = dist;
                    best_x = cand_x;
                    best_y = cand_y;
                }
            }
            
            cand_x = cur_x;
            cand_y = cur_y + 1;
            if (is_walkable(cand_x, cand_y))
            {
                dx = (cand_x > player_x) ? (cand_x - player_x) : (player_x - cand_x);
                dy = (cand_y > player_y) ? (cand_y - player_y) : (player_y - cand_y);
                dist = dx + dy;
                if (dist < best_dist)
                {
                    best_dist = dist;
                    best_x = cand_x;
                    best_y = cand_y;
                }
            }
            
            cand_x = cur_x - 1;
            cand_y = cur_y;
            if (is_walkable(cand_x, cand_y))
            {
                dx = (cand_x > player_x) ? (cand_x - player_x) : (player_x - cand_x);
                dy = (cand_y > player_y) ? (cand_y - player_y) : (player_y - cand_y);
                dist = dx + dy;
                if (dist < best_dist)
                {
                    best_dist = dist;
                    best_x = cand_x;
                    best_y = cand_y;
                }
            }
            
            ghost_x[i] = best_x;
            ghost_y[i] = best_y;
            
            if (ghost_x[i] == player_x && ghost_y[i] == player_y)
            {
                draw_maze(player_x, player_y, ghost_x, ghost_y);
                my_print("\n");
                my_print(red_str);
                my_print(bold_str);
                my_print("Oh no! A ghost got you. Game Over!\n");
                my_print(reset_str);
                return 0;
            }
        }
    }
    
    return 0;
}
