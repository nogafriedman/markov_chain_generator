#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60

#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20

#define USAGE_ERROR "Usage: The program receives 2 arguments:\n"\
"seed (positive int), num of paths to create (positive int).\n"
#define ARG_NUM 3
#define BASE 10
#define SEED argv[1]
#define PATHS argv[2]
#define NEWLINE "\n"
#define TRUE 1
#define FALSE 0

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell {
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladder in case
    // there is one from this square
    int snake_to;  // snake_to represents the jump of the snake in case
    // there is one from this square
    // both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

/** Error handler **/
static int handle_error(char *error_msg, MarkovChain **database)
{
    printf("%s", error_msg);
    if (database != NULL)
    {
        free_markov_chain(database);
    }
    return EXIT_FAILURE;
}

static int create_board(Cell *cells[BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cells[i] = malloc(sizeof(Cell));
        if (cells[i] == NULL)
        {
            for (int j = 0; j < i; j++) {
                free(cells[j]);
            }
            handle_error(ALLOCATION_ERROR_MASSAGE,NULL);
            return EXIT_FAILURE;
        }
        *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
    }

    for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
    {
        int from = transitions[i][0];
        int to = transitions[i][1];
        if (from < to)
        {
            cells[from - 1]->ladder_to = to;
        }
        else
        {
            cells[from - 1]->snake_to = to;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database(MarkovChain *markov_chain)
{
    Cell* cells[BOARD_SIZE];
    if(create_board(cells) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    MarkovNode *from_node = NULL, *to_node = NULL;
    size_t index_to;
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        add_to_database(markov_chain, cells[i]);
    }

    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        from_node = get_node_from_database(markov_chain,cells[i])->data;

        if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
        {
            index_to = MAX(cells[i]->snake_to,cells[i]->ladder_to) - 1;
            to_node = get_node_from_database(markov_chain, cells[index_to])
                    ->data;
            add_node_to_counter_list(from_node, to_node, markov_chain);
        }
        else
        {
            for (int j = 1; j <= DICE_MAX; j++)
            {
                index_to = ((Cell*) (from_node->data))->number + j - 1;
                if (index_to >= BOARD_SIZE)
                {
                    break;
                }
                to_node = get_node_from_database(markov_chain, cells[index_to])
                        ->data;
                add_node_to_counter_list(from_node, to_node, markov_chain);
            }
        }
    }
    // free temp arr
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        free(cells[i]);
    }
    return EXIT_SUCCESS;
}

static bool is_last_cell (void *data_ptr)
{
  Cell *cell = (Cell *)data_ptr;
  if (cell->number == BOARD_SIZE)
  {
    return TRUE;
  }
  return FALSE;
}

static void print_cell (void *data)
{
  Cell *cur_cell = data;
  printf ("[%d]", cur_cell->number);
  if (cur_cell->ladder_to != EMPTY)
  {
    printf ("-ladder to %d", cur_cell->ladder_to);
  }
  if (cur_cell->snake_to != EMPTY)
  {
    printf ("-snake to %d", cur_cell->snake_to);
  }
  if (!is_last_cell (cur_cell))
  {
    printf (" -> ");
  }
}

static void *copy_cell (void *data_ptr) // copy struct Cell
{
  Cell *new_cell = malloc(sizeof(Cell));
  new_cell->number = ((Cell *)data_ptr)->number;
  new_cell->ladder_to = ((Cell *)data_ptr)->ladder_to;
  new_cell->snake_to = ((Cell *)data_ptr)->snake_to;
  return (void *)new_cell;
}

static int comp_cells (void *ptr1, void *ptr2)
{
  Cell *first_cell = ptr1;
  Cell *second_cell = ptr2;
  return (first_cell->number - second_cell->number);
}

static void free_cell (void *ptr)
{
  free (ptr);
}

static void create_path (MarkovChain *markov_chain, int i)
{
  printf ("Random Walk %d: ", i);
  Node *first_cell = markov_chain->database->first;
  generate_random_sequence (markov_chain, first_cell->data,
  MAX_GENERATION_LENGTH);
  printf (NEWLINE);
}

static void init_cell_functions (MarkovChain *markov_chain)
{
  markov_chain->print_func = print_cell;
  markov_chain->is_last = is_last_cell;
  markov_chain->comp_func = comp_cells;
  markov_chain->free_data = free_cell;
  markov_chain->copy_func = copy_cell;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
  if (argc != ARG_NUM) // 3 args
  {
    fprintf (stderr, USAGE_ERROR);
    return EXIT_FAILURE;
  }
  int seed = strtol (SEED, NULL, BASE);
  srand (seed);
  int num_paths = strtol (PATHS, NULL, BASE);
  LinkedList *linked_list = malloc (sizeof (LinkedList));
  if (!linked_list)
  {
    handle_error (ALLOCATION_ERROR_MASSAGE, NULL);
    return EXIT_FAILURE;
  }
  linked_list->first = NULL, linked_list->last = NULL; linked_list->size = 0;
  MarkovChain* markov_chain = malloc (sizeof (MarkovChain));
  if (!markov_chain)
  {
    handle_error(ALLOCATION_ERROR_MASSAGE, &markov_chain);
    return EXIT_FAILURE;
  }
  markov_chain->database = linked_list;
  init_cell_functions (markov_chain);
  int res = fill_database (markov_chain);
  if (res == EXIT_FAILURE)
  {
    handle_error (ALLOCATION_ERROR_MASSAGE, &markov_chain);
    return EXIT_FAILURE;
  }
  for (int i = 1; i <= num_paths; i++) // generate requested number of paths
  {
    create_path (markov_chain, i);
  }
  free_markov_chain (&markov_chain);
  return EXIT_SUCCESS;
}