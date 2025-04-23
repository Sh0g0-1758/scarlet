char clear_screen_str[8] = { 27, '[', '2', 'J', 27, '[', 'H', '\0' };
char reset_str[5]        = { 27, '[', '0', 'm', '\0' };
char bold_str[5]         = { 27, '[', '1', 'm', '\0' };
char blue_str[8]         = { 27, '[', '1', ';', '3', '4', 'm', '\0' };
char yellow_str[8]       = { 27, '[', '1', ';', '3', '3', 'm', '\0' };
char red_str[8]          = { 27, '[', '1', ';', '3', '1', 'm', '\0' };
char white_str[8]        = { 27, '[', '1', ';', '3', '7', 'm', '\0' };
char green_str[8]        = { 27, '[', '1', ';', '3', '2', 'm', '\0' };
char cyan_str[8]         = { 27, '[', '1', ';', '3', '6', 'm', '\0' };
char black_bg_str[8]     = { 27, '[', '4', '0', 'm', '\0', '\0', '\0' };
char white_bg_str[8]     = { 27, '[', '4', '7', 'm', '\0', '\0', '\0' };

int putchar(int c);
int getchar(void);
void *malloc(unsigned int size);
void free(void *ptr);

int BOARD_SIZE = 8;
int WHITE = 0;
int BLACK = 1;

char WHITE_KING = 'K';
char WHITE_QUEEN = 'Q';
char WHITE_ROOK = 'R';
char WHITE_BISHOP = 'B';
char WHITE_KNIGHT = 'N';
char WHITE_PAWN = 'P';
char BLACK_KING = 'k';
char BLACK_QUEEN = 'q';
char BLACK_ROOK = 'r';
char BLACK_BISHOP = 'b';
char BLACK_KNIGHT = 'n';
char BLACK_PAWN = 'p';
char EMPTY = ' ';

char board[8][8];
int current_player;
int game_over;
int white_king_x;
int white_king_y;
int black_king_x;
int black_king_y;
int check_status;
int winner;

void print(char *s) {
    while (*s) {
        putchar(*s);
        s = s + 1;
    }
}

void print_num(int n) {
    int rev = 0;
    if (n == 0) {
        putchar('0');
        return;
    }
    if (n < 0) {
        putchar('-');
        n = -n;
    }
    while (n > 0) {
        rev = rev * 10 + (n % 10);
        n = n / 10;
    }
    while (rev > 0) {
        putchar((rev % 10) + '0');
        rev = rev / 10;
    }
}

void clear_screen(void) {
    print(clear_screen_str);
}

void init_board(void) {
    int i;
    int j;
    
    for (i = 0; i < BOARD_SIZE; i = i + 1) {
        for (j = 0; j < BOARD_SIZE; j = j + 1) {
            board[i][j] = EMPTY;
        }
    }
    
    board[0][0] = BLACK_ROOK;
    board[0][1] = BLACK_KNIGHT;
    board[0][2] = BLACK_BISHOP;
    board[0][3] = BLACK_QUEEN;
    board[0][4] = BLACK_KING;
    board[0][5] = BLACK_BISHOP;
    board[0][6] = BLACK_KNIGHT;
    board[0][7] = BLACK_ROOK;
    
    for (j = 0; j < BOARD_SIZE; j = j + 1) {
        board[1][j] = BLACK_PAWN;
    }
    
    for (j = 0; j < BOARD_SIZE; j = j + 1) {
        board[6][j] = WHITE_PAWN;
    }
    
    board[7][0] = WHITE_ROOK;
    board[7][1] = WHITE_KNIGHT;
    board[7][2] = WHITE_BISHOP;
    board[7][3] = WHITE_QUEEN;
    board[7][4] = WHITE_KING;
    board[7][5] = WHITE_BISHOP;
    board[7][6] = WHITE_KNIGHT;
    board[7][7] = WHITE_ROOK;
    
    white_king_x = 4;
    white_king_y = 7;
    black_king_x = 4;
    black_king_y = 0;
    
    current_player = WHITE;
    game_over = 0;
    check_status = 0;
    winner = -1;
}

void draw_board(void) {
    int i;
    int j;
    int is_dark;
    
    clear_screen();
    print("  Chess Game - 2 Player\n\n");
    
    print("   ");
    for (j = 0; j < BOARD_SIZE; j = j + 1) {
        putchar('a' + j);
        putchar(' ');
    }
    putchar('\n');
    
    for (i = 0; i < BOARD_SIZE; i = i + 1) {
        print_num(BOARD_SIZE - i);
        print("  ");
        
        for (j = 0; j < BOARD_SIZE; j = j + 1) {
            is_dark = (i + j) % 2;
            
            if (is_dark) {
                print(black_bg_str);
            } else {
                print(white_bg_str);
            }
            
            if (board[i][j] == EMPTY) {
                print(" ");
            } else if (board[i][j] >= 'a' && board[i][j] <= 'z') {
                print(blue_str);
                putchar(board[i][j]);
            } else {
                print(yellow_str);
                putchar(board[i][j]);
            }
            
            print(reset_str);
            putchar(' ');
        }
        
        putchar('\n');
    }
    
    print("\nCurrent player: ");
    if (current_player == WHITE) {
        print(yellow_str);
        print("WHITE");
    } else {
        print(blue_str);
        print("BLACK");
    }
    print(reset_str);
    
    if (check_status == 1) {
        print("\n");
        print(red_str);
        print("CHECK!");
        print(reset_str);
    }
    
    print("\n\nEnter move (e.g. e2e4) or 'q' to quit: ");
}

int is_white_piece(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

int is_black_piece(char piece) {
    return piece >= 'a' && piece <= 'z';
}

int is_valid_position(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

int is_empty(int row, int col) {
    return board[row][col] == EMPTY;
}

int is_enemy(int row, int col, int player) {
    if (player == WHITE) {
        return is_black_piece(board[row][col]);
    } else {
        return is_white_piece(board[row][col]);
    }
}

int is_valid_pawn_move(int from_row, int from_col, int to_row, int to_col) {
    int direction;
    int starting_row;
    
    if (current_player == WHITE) {
        direction = -1;
        starting_row = 6;
    } else {
        direction = 1;
        starting_row = 1;
    }
    
    if (from_col == to_col) {
        if (to_row == from_row + direction && is_empty(to_row, to_col)) {
            return 1;
        }
        
        if (from_row == starting_row && 
                to_row == from_row + 2 * direction && 
                is_empty(from_row + direction, from_col) && 
                is_empty(to_row, to_col)) {
            return 1;
        }
    } 
    else if ((to_col == from_col - 1 || to_col == from_col + 1) && 
                     to_row == from_row + direction) {
        if (is_enemy(to_row, to_col, current_player)) {
            return 1;
        }
    }
    
    return 0;
}

int is_valid_rook_move(int from_row, int from_col, int to_row, int to_col) {
    int i;
    int step;
    
    if (from_row != to_row && from_col != to_col) {
        return 0;
    }
    
    if (from_row == to_row) {
        step = (to_col > from_col) ? 1 : -1;
        for (i = from_col + step; i != to_col; i = i + step) {
            if (!is_empty(from_row, i)) {
                return 0;
            }
        }
    }
    else {
        step = (to_row > from_row) ? 1 : -1;
        for (i = from_row + step; i != to_row; i = i + step) {
            if (!is_empty(i, from_col)) {
                return 0;
            }
        }
    }
    
    return is_empty(to_row, to_col) || is_enemy(to_row, to_col, current_player);
}

int is_valid_knight_move(int from_row, int from_col, int to_row, int to_col) {
    int row_diff = (to_row > from_row) ? (to_row - from_row) : (from_row - to_row);
    int col_diff = (to_col > from_col) ? (to_col - from_col) : (from_col - to_col);
    
    if ((row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2)) {
        return is_empty(to_row, to_col) || is_enemy(to_row, to_col, current_player);
    }
    
    return 0;
}

int is_valid_bishop_move(int from_row, int from_col, int to_row, int to_col) {
    int row_diff = (to_row > from_row) ? (to_row - from_row) : (from_row - to_row);
    int col_diff = (to_col > from_col) ? (to_col - from_col) : (from_col - to_col);
    int row_step;
    int col_step;
    int i;
    int j;
    
    if (row_diff != col_diff) {
        return 0;
    }
    
    row_step = (to_row > from_row) ? 1 : -1;
    col_step = (to_col > from_col) ? 1 : -1;
    
    i = from_row + row_step;
    j = from_col + col_step;
    while (i != to_row && j != to_col) {
        if (!is_empty(i, j)) {
            return 0;
        }
        i = i + row_step;
        j = j + col_step;
    }
    
    return is_empty(to_row, to_col) || is_enemy(to_row, to_col, current_player);
}

int is_valid_queen_move(int from_row, int from_col, int to_row, int to_col) {
    return is_valid_rook_move(from_row, from_col, to_row, to_col) || 
                 is_valid_bishop_move(from_row, from_col, to_row, to_col);
}

int is_valid_king_move(int from_row, int from_col, int to_row, int to_col) {
    int row_diff = (to_row > from_row) ? (to_row - from_row) : (from_row - to_row);
    int col_diff = (to_col > from_col) ? (to_col - from_col) : (from_col - to_col);
    
    if (row_diff <= 1 && col_diff <= 1) {
        return is_empty(to_row, to_col) || is_enemy(to_row, to_col, current_player);
    }
    
    return 0;
}

int is_valid_move(int from_row, int from_col, int to_row, int to_col) {
    char piece;
    
    if (!is_valid_position(from_row, from_col) || !is_valid_position(to_row, to_col)) {
        return 0;
    }
    
    piece = board[from_row][from_col];
    if (piece == EMPTY) {
        return 0;
    }
    
    if ((current_player == WHITE && !is_white_piece(piece)) ||
            (current_player == BLACK && !is_black_piece(piece))) {
        return 0;
    }
    
    if ((current_player == WHITE && is_white_piece(board[to_row][to_col])) ||
            (current_player == BLACK && is_black_piece(board[to_row][to_col]))) {
        return 0;
    }
    
    if (piece == WHITE_PAWN || piece == BLACK_PAWN) {
        return is_valid_pawn_move(from_row, from_col, to_row, to_col);
    } else if (piece == WHITE_ROOK || piece == BLACK_ROOK) {
        return is_valid_rook_move(from_row, from_col, to_row, to_col);
    } else if (piece == WHITE_KNIGHT || piece == BLACK_KNIGHT) {
        return is_valid_knight_move(from_row, from_col, to_row, to_col);
    } else if (piece == WHITE_BISHOP || piece == BLACK_BISHOP) {
        return is_valid_bishop_move(from_row, from_col, to_row, to_col);
    } else if (piece == WHITE_QUEEN || piece == BLACK_QUEEN) {
        return is_valid_queen_move(from_row, from_col, to_row, to_col);
    } else if (piece == WHITE_KING || piece == BLACK_KING) {
        return is_valid_king_move(from_row, from_col, to_row, to_col);
    }
    
    return 0;
}

void update_king_position(int from_row, int from_col, int to_row, int to_col) {
    char piece = board[from_row][from_col];
    
    if (piece == WHITE_KING) {
        white_king_x = to_col;
        white_king_y = to_row;
    } else if (piece == BLACK_KING) {
        black_king_x = to_col;
        black_king_y = to_row;
    }
}

int is_king_in_check(int player) {
    int i;
    int j;
    int king_row;
    int king_col;
    int enemy_player;
    
    if (player == WHITE) {
        king_row = white_king_y;
        king_col = white_king_x;
        enemy_player = BLACK;
    } else {
        king_row = black_king_y;
        king_col = black_king_x;
        enemy_player = WHITE;
    }
    
    for (i = 0; i < BOARD_SIZE; i = i + 1) {
        for (j = 0; j < BOARD_SIZE; j = j + 1) {
            if ((enemy_player == WHITE && is_white_piece(board[i][j])) ||
                    (enemy_player == BLACK && is_black_piece(board[i][j]))) {
                
                current_player = enemy_player;
                if (is_valid_move(i, j, king_row, king_col)) {
                    current_player = player;
                    return 1;
                }
                current_player = player;
            }
        }
    }
    
    return 0;
}

int is_king_captured(void) {
    int i;
    int j;
    int white_king_found;
    int black_king_found;
    
    white_king_found = 0;
    black_king_found = 0;
    
    for (i = 0; i < BOARD_SIZE; i = i + 1) {
        for (j = 0; j < BOARD_SIZE; j = j + 1) {
            if (board[i][j] == WHITE_KING) {
                white_king_found = 1;
            } else if (board[i][j] == BLACK_KING) {
                black_king_found = 1;
            }
        }
    }
    
    if (!white_king_found) {
        winner = BLACK;
        return 1;
    }
    
    if (!black_king_found) {
        winner = WHITE;
        return 1;
    }
    
    return 0;
}

int would_be_in_check(int from_row, int from_col, int to_row, int to_col) {
    char temp_piece;
    int result;
    
    temp_piece = board[to_row][to_col];
    
    board[to_row][to_col] = board[from_row][from_col];
    board[from_row][from_col] = EMPTY;
    
    result = is_king_in_check(current_player);
    
    board[from_row][from_col] = board[to_row][to_col];
    board[to_row][to_col] = temp_piece;
    
    return result;
}

int is_checkmate(void) {
    int i;
    int j;
    int k;
    int l;
    
    if (!check_status) {
        return 0;
    }
    
    for (i = 0; i < BOARD_SIZE; i = i + 1) {
        for (j = 0; j < BOARD_SIZE; j = j + 1) {
            if ((current_player == WHITE && is_white_piece(board[i][j])) ||
                    (current_player == BLACK && is_black_piece(board[i][j]))) {
                
                for (k = 0; k < BOARD_SIZE; k = k + 1) {
                    for (l = 0; l < BOARD_SIZE; l = l + 1) {
                        if (is_valid_move(i, j, k, l) && !would_be_in_check(i, j, k, l)) {
                            return 0;
                        }
                    }
                }
            }
        }
    }
    
    winner = (current_player == WHITE) ? BLACK : WHITE;
    return 1;
}

void make_move(int from_row, int from_col, int to_row, int to_col) {
    char piece = board[from_row][from_col];
    char captured = board[to_row][to_col];
    
    if (captured == WHITE_KING) {
        winner = BLACK;
        game_over = 1;
    } else if (captured == BLACK_KING) {
        winner = WHITE;
        game_over = 1;
    }
    
    board[to_row][to_col] = piece;
    board[from_row][from_col] = EMPTY;
    
    update_king_position(from_row, from_col, to_row, to_col);
    
    if (piece == WHITE_PAWN && to_row == 0) {
        board[to_row][to_col] = WHITE_QUEEN;
    } else if (piece == BLACK_PAWN && to_row == 7) {
        board[to_row][to_col] = BLACK_QUEEN;
    }
    
    current_player = (current_player == WHITE) ? BLACK : WHITE;
    
    check_status = is_king_in_check(current_player);
    
    if (check_status && is_checkmate()) {
        game_over = 1;
    }
}

int process_move(void) {
    char input[5];
    int from_col;
    int from_row;
    int to_col;
    int to_row;
    int i;
    int valid_move;
    
    i = 0;
    while (i < 4) {
        input[i] = getchar();
        if (input[i] == 'q' || input[i] == 'Q') {
            return 0;
        }
        i = i + 1;
    }
    input[i] = '\0';
    
    getchar();
    
    from_col = input[0] - 'a';
    from_row = BOARD_SIZE - (input[1] - '0');
    to_col = input[2] - 'a';
    to_row = BOARD_SIZE - (input[3] - '0');
    
    valid_move = is_valid_move(from_row, from_col, to_row, to_col);
    
    if (valid_move && would_be_in_check(from_row, from_col, to_row, to_col)) {
        print(red_str);
        print("\nIllegal move! Would leave/put your king in check. Press any key to continue...");
        print(reset_str);
        getchar();
        return 1;
    }
    
    if (valid_move) {
        make_move(from_row, from_col, to_row, to_col);
        return 1;
    } else {
        print(red_str);
        print("\nInvalid move! Press any key to continue...");
        print(reset_str);
        getchar();
        return 1;
    }
}

void display_result(void) {
    clear_screen();
    draw_board();
    
    print("\n\n");
    print(bold_str);
    
    if (winner == WHITE) {
        print(yellow_str);
        print("CHECKMATE! White wins!");
    } else if (winner == BLACK) {
        print(blue_str);
        print("CHECKMATE! Black wins!");
    } else {
        print(white_str);
        print("Game ended.");
    }
    
    print(reset_str);
    print("\n\nPress any key to exit...");
    getchar();
}

int main(void) {
    init_board();
    
    while (!game_over) {
        draw_board();
        
        if (!process_move()) {
            game_over = 1;
        }
        
        if (is_king_captured()) {
            game_over = 1;
        }
    }
    
    display_result();
    
    return 0;
}