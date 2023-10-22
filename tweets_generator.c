#include "markov_chain.h"
#include <string.h>
#define ARG_NUM 5
#define ARG_NUM_ALL_FILE 4 // num of words to read unspecified, read all
#define USAGE_ERROR "Usage: The program receives 4/3 arguments:\n"\
"seed (positive int), num of tweets to create (positive int), file path, "\
"num of words to read from file (optional).\n"
#define SEED argv[1]
#define NUM_TWEETS argv[2]
#define FILE_PATH argv[3]
#define NUM_WORDS argv[4]
#define WORDS_UNSPECIFIED -1
#define INVALID_FILE_MSG "ERROR: File path is invalid."
#define BASE 10
#define FALSE 0
#define TRUE 1
#define DELIM " \n"
#define TWEET_MSG "Tweet %d: "
#define NEWLINE "\n"
#define MAX_CHAIN_LEN 20
#define MAX_LINE_LEN 1001

/**
 * checks if a certain string ends with a period.
 * @param data pointer to void*
 * @return true (1) if ends with a period, false (0) if not
 */
static bool is_last_str (void *data_ptr) // checks if last word in markovchain
{
  char *word = (char *)data_ptr;
  int i = 0;
  while (strcmp(&word[i+1], "\0") != 0)
  {
    i++;
  }
  if (strcmp(&word[i], ".") == 0)
  {
    return TRUE;
  }
  return FALSE;
}

/**
 * prints a string.
 * @param data pointer to void*
 */
static void print_str (void *data_ptr)
{
  char *word = (char *) data_ptr;
  printf ("%s", word);
  if (!is_last_str(word))
  {
    printf (" ");
  }
}
/**
 * compares two strings.
 * @param data pointer to void*
 * @return negative if ptr2 > ptr1, positive if pt1 > ptr2, 0 if equal
 */
static int comp_str (void *ptr1, void *ptr2)
{
  char *casted_ptr1 = (char *) ptr1; // type cast
  char *casted_ptr2 = (char *) ptr2;
  return strcmp (casted_ptr1, casted_ptr2);
}

/**
 * frees a string ptr.
 * @param data pointer to void*
 */
static void free_str (void *ptr)
{
  free(ptr);
}

/**
 * gets a pointer to a string and makes a copy of it.
 * @param data pointer to void*
 * @return void* of the copied string, NULL in case of allocation failure
 */
static void* copy_str (void *ptr)
{
  char *converted_ptr = (char *) ptr; // type cast
  char *string = malloc (strlen(converted_ptr) + 1);
  if (!string)
  {
    return NULL;
  }
  strcpy(string, converted_ptr);
  return (void*)string;
}

static Node *handle_database (MarkovChain *markov_chain, void *data_ptr,
                           Node *last_node_added)
{
  Node *node = add_to_database (markov_chain, data_ptr);
  if (!node)
  {
    return NULL;
  }
  if (last_node_added == NULL)
  {
    return node;
  }
  int res = add_node_to_counter_list (last_node_added->data, node->data,
                                      markov_chain);
  if (!res) // memory allocation failure
  {
    return NULL;
  }
  return node;
}

/**
 * Receives a file, a number of words to read, and a pointer to Markov data
 * structure, reads the words from the file and fills the data structure.
 * @param fp file path
 * @param words_to_read positive integer
 * @param markov_chain data structure
 * @return EXIT_FAILURE in case of memory allocation failure, otherwise
 * EXIT_SUCCESS
 */
static int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain) {
  int words_countdown = words_to_read;
  char line[MAX_LINE_LEN];
  char *cur_word;
  Node *last_node_added = NULL;
  while (fgets(line, MAX_LINE_LEN, fp))
  {
    cur_word = strtok(line, DELIM);
    if (!cur_word)
    {
      continue;
    }
    while (cur_word && (words_countdown != 0))
    {
      last_node_added = handle_database(markov_chain, cur_word,
                                        last_node_added);
      if (last_node_added == NULL) // allocation failure
      {
        return EXIT_FAILURE;
      }
      cur_word = strtok (NULL, DELIM);
      words_countdown--;
    }
  }
  return EXIT_SUCCESS;
}

static void init_char_functions (MarkovChain *markov_chain)
{
  markov_chain->copy_func = copy_str;
  markov_chain->free_data = free_str;
  markov_chain->is_last = is_last_str;
  markov_chain->comp_func = comp_str;
  markov_chain->print_func = print_str;
}

static void create_tweets (MarkovChain *markov_chain, char *tweets_arg)
{
  int tweet_count = 0;
  int num_tweets = strtol (tweets_arg, NULL, BASE);
  while (tweet_count < num_tweets)
  {
    tweet_count++;
    printf (TWEET_MSG, tweet_count);
    MarkovNode *first_node = get_first_random_node (markov_chain);
    generate_random_sequence (markov_chain, first_node, MAX_CHAIN_LEN);
    printf (NEWLINE);
  }
}

static void handle_failure (FILE *file, LinkedList *linked_list, MarkovChain
*markov_chain)
{
  if (!linked_list)
  {
    free (linked_list);
    linked_list = NULL;
  }
  if (!markov_chain)
  {
    free (markov_chain); // free markov chain
    markov_chain = NULL;
  }
  fclose (file);
  fprintf (stdout, ALLOCATION_ERROR_MASSAGE);
}

FILE *init_program (int argc, char *argv[])
{
  int seed = strtol (SEED, NULL, BASE);
  srand (seed);
  if ((argc != ARG_NUM) && argc != ARG_NUM_ALL_FILE)
  {
    fprintf (stderr, USAGE_ERROR);
    return NULL;
  }
  FILE *file_path = fopen (FILE_PATH, "r");
  if (!file_path)
  {
    fprintf (stderr, INVALID_FILE_MSG);
    return NULL;
  }
  return file_path;
}

MarkovChain *init_data_types (LinkedList *linked_list, FILE *file_path)
{
  if (!linked_list)
  {
    handle_failure (file_path, NULL, NULL);
    return NULL;
  }
  linked_list->first = NULL, linked_list->last = NULL; linked_list->size = 0;
  MarkovChain* markov_chain = malloc (sizeof (MarkovChain));
  if (markov_chain)
  {
    init_char_functions (markov_chain);
    markov_chain->database = linked_list;
  }
  else
  {
    handle_failure (file_path, linked_list, NULL);
  }
  return markov_chain;
}

/**
 * @param argc num of arguments: 4 or 5
 * @param argv the following arguments:
 * 1. seed (unsigned int, can assume it's a positive integer)
 * 2. num_tweets (can assume it's a positive integer)
 * 3. path to text corpus (not necessarily valid - might not exist or can't
 * be accessed - then put stderr ERROR: message and exit with EXIT_FAILURE)
 * 4. num_of_words_to_read (if received - can assume it's a positive integer.
 * if none received - read the entire file. no errors for this one)
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main (int argc, char *argv[])
{
  FILE *file_path = init_program (argc, argv);
  if (!file_path)
  {
    return EXIT_FAILURE;
  } // init data types (linked list and markov chain):
  LinkedList *linked_list = malloc (sizeof (LinkedList));
  MarkovChain *markov_chain = init_data_types (linked_list, file_path);
  if (!markov_chain)
  {
    return EXIT_FAILURE;
  }
  int res;
  if (argc == ARG_NUM) // num of words to read was specified
  {
    int int_words = strtol (NUM_WORDS, NULL, BASE);
    res = fill_database (file_path, int_words, markov_chain);
  }
  else // if num of words was not specified - read the entire file
  {
    res = fill_database (file_path, WORDS_UNSPECIFIED, markov_chain);
  }
  if (res == EXIT_FAILURE) // in case of allocation failure
  {
    handle_failure (file_path, linked_list, markov_chain);
    return EXIT_FAILURE;
  } // database has been loaded, start creating tweets:
  create_tweets (markov_chain, NUM_TWEETS);
  fclose(file_path);
  free_markov_chain (&markov_chain);
  return EXIT_SUCCESS;
}