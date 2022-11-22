#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

#define BOARD_SIZE 8       /* board size */
#define ROWS_WITH_PIECES 3 /* number of initial rows with pieces */
#define CELL_EMPTY '.'     /* empty cell character */
#define CELL_BPIECE 'b'    /* black piece character */
#define CELL_WPIECE 'w'    /* white piece character */
#define CELL_BTOWER 'B'    /* black tower character */
#define CELL_WTOWER 'W'    /* white tower character */
#define COST_PIECE 1       /* one piece cost */
#define COST_TOWER 3       /* one tower cost */
#define TREE_DEPTH 3       /* minimax tree depth */
#define COMP_ACTIONS 10    /* number of computed actions */

/* strings required when printing the board */
#define ROW_SEPERATOR "   +---+---+---+---+---+---+---+---+\n"
#define MOVE_SEPERATOR "=====================================\n"
#define TEAM_PIECES 12
#define COLUMNS "     A   B   C   D   E   F   G   H\n"
#define PROGRAM_MOVE "*** "

#define SENTINEL -1          /* returned if piece is outside of the board */
#define CAPTURE_DIST 2       /* the vertical/horizontal capture distance */
#define MOVE_DIST 1          /* the vertical/horizontal move distance */
#define INITIAL_DEPTH 0      /* initial depth of decision tree */
#define BLACK_DIRECTION -1   /* direction black moves in terms of row */
#define WHITE_DIRECTION 1    /* direction white moves in terms of row */
#define MAX_POS_MOVES 4      /* most moves a piece can possibly have */
#define BLACK_WIN 1          /* black wins reference number */
#define WHITE_WIN -1         /* white wins reference number */
#define MOVE_COMPUTED 1      /* indicates if a move played by the program */
#define MOVE_READ 0          /* indicates if a move read by the program */
#define CAPTURE_OCCURS 1     /* a capture did occur */
#define NO_CAPTURE 0         /* a capture did not occur */
#define CHECK_ODDEVEN 2      /* % divisor to check if a number is odd/even */
#define BLACK_MOVE 1         /* F2 value of movenum if it is blacks move */
#define WHITE_MOVE 0         /* F2 value of movenum if it is whites move */
#define INITIAL_MOVE 0       /* Where the move counter should start */
#define PLAY_ONE_MOVE 'A'    /* Instruction if we are to play one move */
#define PLAY_TEN_MOVES 'P'   /* Instruction if we are to play ten moves */
#define DIRECTION_DISTANCE 2 /* The distance between directional tests */
#define CHECK_OVER 1         /* For checking if the game is over */
#define NO_OPTIONS 0         /* For when node is indicative of game over */

/* Note; board will be traversed in row major order */
typedef char board_t[BOARD_SIZE][BOARD_SIZE];
typedef struct
{
    char sourcecol, sourcerow, targetcol, targetrow;
    int movenum;
} move_t;

typedef struct
{
    int i, j, addi, addj;
} direction_t;

typedef struct node decision_node_t;
struct node
{
    board_t board;
    int options, minimax_cost;
    move_t move;
    decision_node_t *next_move;
};

void print_move(board_t board, int programmove, move_t curmove);
void print_board(board_t board);
void fill_board(board_t board);
void print_start(board_t board);
int read_input(board_t board, char *instruction, int *moves);
void update_board(board_t board, move_t curmove, int capture);
int promote_piece(char curpiece, char targetrow);
int convert_to_index(char movenode);
int legal_input(board_t board, move_t curmove, int *capture);
int valid_move(board_t board, move_t curmove, int *capture);
int calculate_cost(board_t board);
int capture_opposition(board_t board, move_t move);

void recursive_addlayers(decision_node_t *node, int move, int tree_depth);
void calculate_options(decision_node_t *node);
void add_options(board_t board, int i, int j, decision_node_t *possible_moves,
                 int *index, int move);
void w_move(board_t board, int i, int j, decision_node_t *possible_moves,
            int *index, int movenum);
void b_move(board_t board, int i, int j, decision_node_t *possible_moves,
            int *index, int movenum);
void WB_move(board_t board, int i, int j, decision_node_t *possible_moves,
             int *index, int movenum);
void test_direction(board_t board, decision_node_t *possible_moves,
                    int *index, int move, direction_t test);
decision_node_t *create_move(board_t board, move_t moves);
void copy_board(board_t board1, board_t board2);
void recursive_free(decision_node_t *root);
int game_over(decision_node_t *root);
void find_move(decision_node_t *root, move_t *best_move);
void recur_fill_costs(decision_node_t *root, int depth);
void propagate_cost(decision_node_t *root);

int play_round(board_t board, int move, int check_gover);

int main(int argc, char *argv[])
{
    /* Stage 0 */
    board_t board;
    char instruction;
    int move = INITIAL_MOVE, moves_to_play;
    fill_board(board);
    print_start(board);
    if (!read_input(board, &instruction, &move))
    {
        return EXIT_FAILURE;
    }
    /* Stage 1 */
    if (instruction == PLAY_ONE_MOVE)
    {
        play_round(board, move++, !CHECK_OVER);
        /* Stage 2 */
    }
    else if (instruction == PLAY_TEN_MOVES)
    {
        for (moves_to_play = COMP_ACTIONS; moves_to_play > 0; moves_to_play--)
        {
            if (!play_round(board, move++, !CHECK_OVER))
            {
                /* The game ended, so return early */
                return EXIT_SUCCESS;
            }
        }
    }
    /* Now, just check if the game ended in the turn we just played */
    play_round(board, move, CHECK_OVER);
    /* All done */
    return EXIT_SUCCESS;
}

int play_round(board_t board, int move, int check_gover)
{
    /* Play a round of the game, return 0 if game over, 1 otherwise */
    int capture = NO_CAPTURE;
    decision_node_t *root = malloc(sizeof(decision_node_t));
    move_t *best_move = malloc(sizeof(move_t));
    copy_board(root->board, board);
    recursive_addlayers(root, move, INITIAL_DEPTH);
    find_move(root, best_move);
    best_move->movenum = move;
    /* Check if the game is over */
    if (game_over(root) == INT_MAX)
    {
        printf("BLACK WIN!\n");
        free(best_move);
        recursive_free(root);
        free(root);
        return 0;
    }
    else if (game_over(root) == INT_MAX)
    {
        printf("WHITE WIN!\n");
        free(best_move);
        recursive_free(root);
        free(root);
        return 0;
    }
    if (check_gover)
    {
        /* Just checking whether the game was over, so can return early */
        free(best_move);
        recursive_free(root);
        free(root);
        return 0;
    }
    /* Ensure legal move as a precaution, and to find capture value */
    if (!legal_input(board, *best_move, &capture))
    {
        free(best_move);
        recursive_free(root);
        free(root);
        printf("ERROR: Illegal action.\n");
        return 0;
    }
    update_board(board, *best_move, capture);
    print_move(board, MOVE_COMPUTED, *best_move);
    free(best_move);
    recursive_free(root);
    free(root);
    return 1;
}

/*-------------------------------------------------------------------*/
/* GAMEPLAY VALIDATION FUNCTIONS */
void fill_board(board_t board)
{
    /* Fill the array with the initial board values */
    int i, j, order = BLACK_MOVE;
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (j == 0)
            {
                /* Use this F2 variable to ensure the pieces alternate each row */
                order++;
                order = order % CHECK_ODDEVEN;
            }
            /* Print the white pieces */
            if (i < ROWS_WITH_PIECES)
            {
                if (j % CHECK_ODDEVEN == order)
                {
                    board[i][j] = CELL_EMPTY;
                }
                else
                {
                    board[i][j] = CELL_WPIECE;
                }
                /* Print the black pieces */
            }
            else if (i >= BOARD_SIZE - ROWS_WITH_PIECES)
            {
                if (j % CHECK_ODDEVEN == order)
                {
                    board[i][j] = CELL_EMPTY;
                }
                else
                {
                    board[i][j] = CELL_BPIECE;
                }
            }
            else
            {
                board[i][j] = CELL_EMPTY;
            }
        }
    }
}

void print_move(board_t board, int programmove, move_t curmove)
{
    /* Print the current move */
    printf(MOVE_SEPERATOR);
    /* Check if we're reading a move or the program is playing it */
    if (programmove)
    {
        printf(PROGRAM_MOVE);
    }
    /* Check if black or white move (given black starts) */
    if (curmove.movenum % CHECK_ODDEVEN == BLACK_MOVE)
    {
        printf("BLACK ACTION #%d: %c%c-%c%c\n", curmove.movenum,
               curmove.sourcecol, curmove.sourcerow,
               curmove.targetcol, curmove.targetrow);
    }
    else
    {
        printf("WHITE ACTION #%d: %c%c-%c%c\n", curmove.movenum,
               curmove.sourcecol, curmove.sourcerow,
               curmove.targetcol, curmove.targetrow);
    }
    printf("BOARD COST: %d\n", calculate_cost(board));
    print_board(board);
}

void print_board(board_t board)
{
    /* Print the current game board */
    int i, j;
    printf(COLUMNS);
    for (i = 0; i < BOARD_SIZE; i++)
    {
        printf(ROW_SEPERATOR);
        /* Print the current row number on the side */
        printf(" %d |", i + 1);
        /* Print every item in that row */
        for (j = 0; j < BOARD_SIZE; j++)
        {
            printf(" %c |", board[i][j]);
        }
        putchar('\n');
    }
    printf(ROW_SEPERATOR);
}

void print_start(board_t board)
{
    /* Print the board we start with */
    printf("BOARD SIZE: %dx%d\n", BOARD_SIZE, BOARD_SIZE);
    printf("#BLACK PIECES: %d\n", TEAM_PIECES);
    printf("#WHITE PIECES: %d\n", TEAM_PIECES);
    print_board(board);
}

int read_input(board_t board, char *instruction, int *moves)
{
    /* Interpret the input, print it all out.
    Return 1 if input is valid, 0 otherwise */
    int programmove = MOVE_READ, capture = NO_CAPTURE;
    move_t curmove;
    curmove.movenum = *moves;
    while (scanf("%c%c-%c%c\n", &curmove.sourcecol, &curmove.sourcerow,
                 &curmove.targetcol, &curmove.targetrow) != EOF)
    {
        curmove.movenum++;
        /* Break if we are at the instruction */
        if (((curmove.sourcecol == 'A') || (curmove.sourcecol == 'P')) &&
            ((curmove.sourcerow == '\n') || (curmove.sourcerow == EOF)))
        {
            break;
        }
        /* Check if input is legal */
        if (!legal_input(board, curmove, &capture))
        {
            return 0;
        }
        update_board(board, curmove, capture);
        print_move(board, programmove, curmove);
    }
    /* The first scan should have reached the instruction, so assign it */
    *instruction = curmove.sourcecol;
    *moves = curmove.movenum;
    return 1;
}

void update_board(board_t board, move_t curmove, int capture)
{
    /* Update the board with the read move; assuming input validity */
    char *curpiece = NULL, *target = NULL, *deleted = NULL;
    int sourcerowi = convert_to_index(curmove.sourcerow),
        sourcecoli = convert_to_index(curmove.sourcecol),
        targetrowi = convert_to_index(curmove.targetrow),
        targetcoli = convert_to_index(curmove.targetcol);
    curpiece = &board[sourcerowi][sourcecoli];
    target = &board[targetrowi][targetcoli];
    /* Update the positions */
    if (promote_piece(*curpiece, curmove.targetrow))
    {
        if (*curpiece == CELL_WPIECE)
        {
            *target = CELL_WTOWER;
        }
        else
        {
            *target = CELL_BTOWER;
        }
    }
    else
    {
        *target = *curpiece;
    }
    *curpiece = CELL_EMPTY;
    if (capture)
    {
        /* Need to delete the piece in between */
        deleted = &board[(sourcerowi + targetrowi) / 2][(sourcecoli + targetcoli) / 2];
        *deleted = CELL_EMPTY;
    }
}

int promote_piece(char curpiece, char targetrow)
{
    /* Check if a piece being moved needs to be promoted to a tower */
    if ((curpiece == 'b') && (targetrow == '1'))
    {
        return 1;
    }
    else if ((curpiece == 'w') && (targetrow == '8'))
    {
        return 1;
    }
    return 0;
}

int convert_to_index(char movenode)
{
    /* Finds the board_t index of a typed move, returning the index if valid
    but returning an impossible SENTINEL if move is outside of the board */
    int move_index;
    /* Subtracting the ASCII value of A from any of the
    chars will give its index (finding ASCII value relative to A) */
    if ((movenode >= 'A') && (movenode <= 'H'))
    {
        move_index = (int)movenode - (int)'A';
        /* Subtract the ASCII value of '1' for the same reason */
    }
    else if ((movenode >= '1') && (movenode <= '8'))
    {
        move_index = (int)movenode - (int)'1';
    }
    else
    {
        /* This move must be invalid, so set the sentinel */
        move_index = SENTINEL;
    }
    return move_index;
}

int legal_input(board_t board, move_t curmove, int *capture)
{
    int sourcerowi = convert_to_index(curmove.sourcerow),
        sourcecoli = convert_to_index(curmove.sourcecol),
        targetrowi = convert_to_index(curmove.targetrow),
        targetcoli = convert_to_index(curmove.targetcol);
    char sourcepiece = board[sourcerowi][sourcecoli],
         targetpiece = board[targetrowi][targetcoli];
    /* Check both positions are on the board */
    if ((sourcerowi == SENTINEL) || (sourcecoli == SENTINEL))
    {
        printf("ERROR: Source cell is outside of the board.\n");
        return 0;
    }
    else if ((targetrowi == SENTINEL) || (targetcoli == SENTINEL))
    {
        printf("ERROR: Target cell is outside of the board.\n");
        return 0;
        /* Check cells are valid */
    }
    else if (sourcepiece == CELL_EMPTY)
    {
        printf("ERROR: Source cell is empty.\n");
        return 0;
    }
    else if (targetpiece != CELL_EMPTY)
    {
        printf("ERROR: Target cell is not empty.\n");
        return 0;
        /* Check source cell has correct piece */
    }
    else if (((curmove.movenum % CHECK_ODDEVEN == WHITE_MOVE) &&
              ((sourcepiece == CELL_BTOWER) ||
               (sourcepiece == CELL_BPIECE))) ||
             ((curmove.movenum % CHECK_ODDEVEN == BLACK_MOVE) &&
              ((sourcepiece == CELL_WTOWER) ||
               (sourcepiece == CELL_WPIECE))))
    {
        printf("ERROR: Source cell holds opponent's piece/tower.\n");
        return 0;
        /* Check the move is a move allowable by the rules */
    }
    else if (!valid_move(board, curmove, capture))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int valid_move(board_t board, move_t curmove, int *capture)
{
    /* Check the validity of a proposed move, ensuring a move or capture */
    int sourcerowi = convert_to_index(curmove.sourcerow),
        sourcecoli = convert_to_index(curmove.sourcecol),
        targetrowi = convert_to_index(curmove.targetrow),
        targetcoli = convert_to_index(curmove.targetcol);
    /* Check if this is a capture or move using the distance of the move */
    if ((abs(sourcerowi - targetrowi) == CAPTURE_DIST) &&
        (abs(sourcecoli - targetcoli) == CAPTURE_DIST))
    {
        if (!capture_opposition(board, curmove))
        {
            printf("ERROR: Illegal action.\n");
            return 0;
        }
        else
        {
            *capture = CAPTURE_OCCURS;
            return 1;
        }
    }
    else if ((abs(sourcerowi - targetrowi) == MOVE_DIST) &&
             (abs(sourcecoli - targetcoli) == MOVE_DIST))
    {
        *capture = NO_CAPTURE;
        return 1;
    }
    else
    {
        /* The move must be invalid if not one of these two possible cases */
        printf("ERROR: Illegal action.\n");
        return 0;
    }
}

int capture_opposition(board_t board, move_t move)
{
    /* Ensure an attempt to capture a piece is capturing an opposing piece */
    int sourcerowi = convert_to_index(move.sourcerow),
        sourcecoli = convert_to_index(move.sourcecol),
        targetrowi = convert_to_index(move.targetrow),
        targetcoli = convert_to_index(move.targetcol);
    /* The piece being captured should be the midpoint of target and source */
    char captured_piece = board[(sourcerowi +
                                 targetrowi) /
                                2][(sourcecoli + targetcoli) / 2];
    /* Black is moving */
    if ((move.movenum % CHECK_ODDEVEN == BLACK_MOVE) &&
        ((captured_piece == CELL_BTOWER) ||
         (captured_piece == CELL_BPIECE)))
    {
        return 0;
        /* White is moving */
    }
    else if ((move.movenum % CHECK_ODDEVEN == WHITE_MOVE) &&
             (((captured_piece == CELL_WTOWER) ||
               (captured_piece == CELL_WPIECE))))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int calculate_cost(board_t board)
{
    /* Calculate the current cost of the board */
    int i, j, cost = 0;
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == CELL_WPIECE)
            {
                cost--;
            }
            else if (board[i][j] == CELL_BPIECE)
            {
                cost++;
            }
            else if (board[i][j] == CELL_WTOWER)
            {
                cost -= COST_TOWER;
            }
            else if (board[i][j] == CELL_BTOWER)
            {
                cost += COST_TOWER;
            }
        }
    }
    return cost;
}

/*-------------------------------------------------------------------*/
/* MOVE FINDING FUNCTIONS */

void recursive_addlayers(decision_node_t *node, int move, int tree_depth)
{
    /* Add depth to the tree in a recursive call at a specified depth */
    int i;
    node->move.movenum = move;
    node->options = NO_OPTIONS;
    if (tree_depth > TREE_DEPTH)
    {
        return;
    }
    calculate_options(node);
    if (node->options != NO_OPTIONS)
    {
        for (i = 0; i < (node->options); i++)
        {
            recursive_addlayers(node->next_move + i, move + 1, tree_depth + 1);
        }
    }
    return;
}

void calculate_options(decision_node_t *node)
{
    /* Fill the array of possible move options for a board */
    decision_node_t *possible_moves = malloc(TEAM_PIECES * MAX_POS_MOVES *
                                             sizeof(decision_node_t));
    int i, j, index = 0;
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            /* Add possibilities */
            if ((node->move.movenum) % CHECK_ODDEVEN == BLACK_MOVE)
            {
                /* It must be black's move */
                if (((node->board[i][j]) == CELL_BPIECE) ||
                    ((node->board[i][j]) == CELL_BTOWER))
                {
                    add_options(node->board, i, j, possible_moves,
                                &index, node->move.movenum);
                }
            }
            else if (((node->board[i][j]) == CELL_WPIECE) ||
                     ((node->board[i][j]) == CELL_WTOWER))
            {
                /* It must be white's move */
                add_options(node->board, i, j, possible_moves,
                            &index, node->move.movenum);
            }
        }
    }
    node->options = index;
    node->next_move = possible_moves;
}

void add_options(board_t board, int i, int j,
                 decision_node_t *possible_moves, int *index, int move)
{
    /* Create the options array so we can set it in our tree */
    char curpiece = board[i][j];
    if (curpiece == CELL_WPIECE)
    {
        /* Handle 'w' piece */
        w_move(board, i, j, possible_moves, index, move);
    }
    else if (curpiece == CELL_BPIECE)
    {
        /* Handle 'b' piece */
        b_move(board, i, j, possible_moves, index, move);
    }
    else if ((curpiece == CELL_WTOWER) || (curpiece == CELL_BTOWER))
    {
        /* Handle 'W' and 'B' piece, which have same move range */
        WB_move(board, i, j, possible_moves, index, move);
    }
}

void w_move(board_t board, int i, int j, decision_node_t *possible_moves,
            int *index, int movenum)
{
    /* Handle the possible directions that the piece 'w' could move */
    direction_t test;
    int addi = WHITE_DIRECTION, addj;
    /* Need to traverse the options clockwise from North East */
    for (addj = WHITE_DIRECTION; addj >= BLACK_DIRECTION;
         addj -= DIRECTION_DISTANCE)
    {
        test.j = j;
        test.i = i;
        test.addj = addj;
        test.addi = addi;
        test_direction(board, possible_moves, index, movenum, test);
    }
}

void b_move(board_t board, int i, int j, decision_node_t *possible_moves,
            int *index, int movenum)
{
    /* Handle the possible directions that the piece 'b' could move */
    direction_t test;
    int addi = BLACK_DIRECTION, addj;
    /* Need to traverse the options clockwise from North East */
    for (addj = WHITE_DIRECTION; addj >= BLACK_DIRECTION;
         addj -= DIRECTION_DISTANCE)
    {
        test.j = j;
        test.i = i;
        test.addj = addj;
        test.addi = addi;
        test_direction(board, possible_moves, index, movenum, test);
    }
}

void WB_move(board_t board, int i, int j, decision_node_t *possible_moves,
             int *index, int movenum)
{
    /* Handle the possible directions that the piece 'B' or 'W' could move */
    direction_t test;
    int addi, addj;
    /* Need to traverse the options clockwise from North East */
    for (addj = WHITE_DIRECTION; addj >= BLACK_DIRECTION;
         addj -= DIRECTION_DISTANCE)
    {
        for (addi = BLACK_DIRECTION; addi <= WHITE_DIRECTION;
             addi += DIRECTION_DISTANCE)
        {
            test.j = j;
            test.i = i;
            test.addj = addj;
            test.addi = addi;
            test_direction(board, possible_moves, index, movenum, test);
        }
    }
}

void test_direction(board_t board, decision_node_t *possible_moves,
                    int *index, int move, direction_t test)
{
    /* Test if a piece can play in this direction, either moving/capturing */
    /* Ensure attempted move is on the board and target cell empty */
    move_t new_move;
    new_move.movenum = move;
    /* Is there a move available? */
    if ((test.i + test.addi >= 0) && (test.i + test.addi < BOARD_SIZE) && (test.j + test.addj >= 0) && (test.j + test.addj < BOARD_SIZE) &&
        (board[test.i + test.addi][test.j + test.addj] == '.'))
    {
        new_move.sourcecol = test.j + 'A';
        new_move.targetcol = test.j + test.addj + 'A';
        new_move.sourcerow = test.i + '1';
        new_move.targetrow = test.i + test.addi + '1';
        possible_moves[(*index)++] = *create_move(board, new_move);
        /* No move, see if we can capture (if on board, near cell wont be empty) */
    }
    else if ((test.i + CAPTURE_DIST * test.addi >= 0) &&
             (test.i + CAPTURE_DIST * test.addi < BOARD_SIZE) &&
             (test.j + CAPTURE_DIST * test.addj < BOARD_SIZE) &&
             (test.j + CAPTURE_DIST * test.addj >= 0) &&
             (board[test.i +
                    CAPTURE_DIST * test.addi][test.j + CAPTURE_DIST * test.addj] == '.'))
    {
        new_move.sourcecol = test.j + 'A';
        new_move.targetcol = test.j + CAPTURE_DIST * test.addj + 'A';
        new_move.sourcerow = test.i + '1';
        new_move.targetrow = test.i + CAPTURE_DIST * test.addi + '1';
        /* Ensure we are capturing the other piece */
        if (!capture_opposition(board, new_move))
        {
            return;
        }
        possible_moves[(*index)++] = *create_move(board, new_move);
    }
}

decision_node_t *create_move(board_t board, move_t moves)
{
    int capture = NO_CAPTURE;
    decision_node_t *newnode = malloc(sizeof(decision_node_t));
    copy_board(newnode->board, board);
    if (abs(moves.sourcecol - moves.targetcol) == CAPTURE_DIST &&
        abs(moves.sourcerow - moves.targetrow) == CAPTURE_DIST)
    {
        capture = CAPTURE_OCCURS;
    }
    update_board(newnode->board, moves, capture);
    newnode->next_move = NULL;
    /* Note: Movenum will be updated at the start of the recursive call */
    newnode->move = moves;
    return newnode;
}

void copy_board(board_t board1, board_t board2)
{
    int i, j;
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            board1[i][j] = board2[i][j];
        }
    }
}

void recursive_free(decision_node_t *root)
{
    /* Recursively free all the memory in a decision tree */
    int i;
    if (root)
    {
        for (i = 0; i < (root->options); i++)
        {
            recursive_free(root->next_move + i);
        }
        free(root->next_move);
    }
}

int game_over(decision_node_t *root)
{
    /* Check if the game is over, returning who won if so */
    if (root->options == NO_OPTIONS)
    {
        /* The game must be over if there are no available moves */
        if (root->move.movenum % CHECK_ODDEVEN == BLACK_MOVE)
        {
            return INT_MAX;
        }
        else
        {
            return INT_MIN;
        }
    }
    else
    {
        return 0;
    }
}

void find_move(decision_node_t *root, move_t *best_move)
{
    /* Play the best move available using the minimax decision tree */
    int i;
    if (game_over(root))
    {
        /* We're not going to find any moves, so can return early */
        root->move.movenum = game_over(root);
        return;
    }
    recur_fill_costs(root, INITIAL_DEPTH);
    propagate_cost(root);
    for (i = 0; i < root->options; i++)
    {
        if ((root->next_move + i)->minimax_cost == root->minimax_cost)
        {
            *best_move = (root->next_move + i)->move;
            break;
        }
    }
}

void recur_fill_costs(decision_node_t *root, int depth)
{
    /* Propagate the costs all the way up the tree from TREE_DEPTH */
    int i;
    /* Ensure the game isnt over */
    if (root->options == NO_OPTIONS)
    {
        root->minimax_cost = game_over(root);
        return;
    }
    /* At TREE_DEPTH-1 we need to find cost of all TREE_DEPTH child nodes */
    if (depth == TREE_DEPTH - 1)
    {
        for (i = 0; i < root->options; i++)
        {
            if (game_over(root->next_move + i))
            {
                (root->next_move + i)->minimax_cost =
                    game_over(root->next_move + i);
                continue;
            }
            (root->next_move + i)->minimax_cost =
                calculate_cost((root->next_move + i)->board);
        }
        propagate_cost(root);
        return;
        /* Recursively propagate costs up through the non TREE_DEPTH nodes */
    }
    else
    {
        for (i = 0; i < root->options; i++)
        {
            recur_fill_costs(root->next_move + i, depth + 1);
        }
        propagate_cost(root);
    }
}

void propagate_cost(decision_node_t *root)
{
    /* Return the max/min cost for the children of the current node */
    int propagated_cost, i;
    if (root->move.movenum % CHECK_ODDEVEN == BLACK_MOVE)
    {
        propagated_cost = INT_MIN;
        /* It must be black's move, so find the max */
        for (i = 0; i < root->options; i++)
        {
            if ((root->next_move + i)->minimax_cost > propagated_cost)
            {
                propagated_cost = (root->next_move + i)->minimax_cost;
            }
        }
    }
    else
    {
        /* White's move, so find the min */
        propagated_cost = INT_MAX;
        for (i = 0; i < root->options; i++)
        {
            if ((root->next_move + i)->minimax_cost < propagated_cost)
            {
                propagated_cost = (root->next_move + i)->minimax_cost;
            }
        }
    }
    root->minimax_cost = propagated_cost;
}
