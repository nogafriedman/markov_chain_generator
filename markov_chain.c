#include "markov_chain.h"
#include <string.h>
#include <stdlib.h>
#define TRUE 1
#define FALSE 0
#define NEWLINE "\n"

MarkovNode* init_node (void *cur_word)
{
  MarkovNode *cur_markov_node = malloc (sizeof (MarkovNode));
  if (cur_markov_node == NULL) // if memory allocation failed
  {
    return NULL;
  }
  cur_markov_node->counter_list = malloc (sizeof (NextNodeCounter));
  if (cur_markov_node->counter_list == NULL) // if memory allocation failed
  {
    return NULL;
  }
  cur_markov_node->counter_list->markov_node = NULL;
  cur_markov_node->counter_list->frequency = 0;
  cur_markov_node->data = cur_word;
  cur_markov_node->counter = 0;
  cur_markov_node->sum_freq = 0;
  return cur_markov_node;
}

int get_random_number (int max_number)
{
  return rand() % max_number;
}

MarkovNode* get_first_random_node (MarkovChain *markov_chain)
{
  while (TRUE) // loop until a valid word is found
  {
    int i = get_random_number (markov_chain->database->size);
    int count = 1;
    Node* cur_word = markov_chain->database->first;
    while (count <= i) // loop through markov chain and find the i'th word
    {
      Node* next_word = cur_word->next;
      cur_word = next_word;
      count++;
    }
    void* data = cur_word->data->data;
    int res = markov_chain->is_last(data);
    if (res) // true = last word
    {
      continue; // get another random word.
    }
    else // if word is valid - not a last word in a sentence
    {
      return cur_word->data;
    }
  }
}

MarkovNode* get_next_random_node (MarkovNode *state_struct_ptr)
{
  int num = get_random_number (state_struct_ptr->sum_freq);
  int frq_counter = 0;
  for (int i = 0; i <= state_struct_ptr->counter; i++) // run on cells
  { // run on frequencies
    frq_counter += ((state_struct_ptr->counter_list)[i]).frequency;
    if (num < frq_counter)
    {
      return (state_struct_ptr->counter_list)[i].markov_node;
    }
  }
  return NULL;
}

void generate_random_sequence (MarkovChain *markov_chain, MarkovNode
*first_node, int max_length)
{
  while (first_node == NULL)
  { // get a new random node in case the one received is Null
    first_node = get_first_random_node (markov_chain);
  }
  markov_chain->print_func (first_node->data);
  int print_counter = 1; // how many words have been printed
  MarkovNode *cur = first_node;
  while (print_counter < max_length && !markov_chain->is_last (cur->data))
  {
    cur = get_next_random_node (cur);
    markov_chain->print_func (cur->data);
    print_counter++;
  }
}

bool add_node_to_counter_list (MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain)
{
  int flag = FALSE;
  int i = 0;
  while (i + 1 <= first_node->counter) // traverse counter_list
  {
    if ((first_node->counter_list + i)->markov_node != NULL)
    {
      if (markov_chain->comp_func
      ((first_node->counter_list + i)->markov_node->data,
       second_node->data) == 0) // word is already in list
      {
        (first_node->counter_list + i)->frequency++;
        (first_node->sum_freq)++;
        flag = TRUE;
        break;
      }
      i++;
    }
  }
  if (flag == FALSE) // word is not yet in counter_list - add it:
  {
    NextNodeCounter *temp = realloc (first_node->counter_list,
                                     (first_node->counter + 1) * (sizeof
                                     (NextNodeCounter)));
    if (temp == NULL) // if realloc failed
    {
      return FALSE;
    }
    int index = first_node->counter;
    first_node->counter_list = temp;
    first_node->counter_list[index].markov_node = second_node;
    first_node->counter_list[index].frequency = 1;
    first_node->counter++;
    first_node->sum_freq++;
  }
  return TRUE;
}

Node* get_node_from_database (MarkovChain *markov_chain, void *data_ptr)
{
  Node *cur = markov_chain->database->first;
  while (cur != NULL) // traverse linked list, check if word is there
  {
    if (markov_chain->comp_func (data_ptr, cur->data->data)) // not a match:
    {
      cur = cur->next; // check the next word
      continue;
    } // if there was a match: word is in list
    return cur;
  }
  return NULL; // finished traversing without finding a match.
}

Node* add_to_database (MarkovChain *markov_chain, void *data_ptr)
{
  // check if node is already in database:
  Node *node = get_node_from_database (markov_chain, data_ptr);
  if (node) // if node is already in database:
  {
    return node;
  } // word is not in database - add it:
  void *data = markov_chain->copy_func (data_ptr);
  if (!data)
  {
    return NULL;
  }
  MarkovNode *new_markov_node = init_node (data);
  if (!new_markov_node)
  {
    return NULL;
  }
  int fail = add(markov_chain->database, new_markov_node);
  if (fail)
  {
    return NULL;
  }
  return markov_chain->database->last;
}

void free_markov_chain (MarkovChain **p_markov_chain)
{
  Node *cur = (*(*(p_markov_chain))).database->first; // head of linked list
  while (cur != NULL) // traverse through list and free node node:
  {
    Node *next_node = cur->next;
    (*p_markov_chain)->free_data (cur->data->data);
    cur->data->data = NULL;
    free (cur->data->counter_list);
    cur->data->counter_list = NULL;
    free (cur->data);
    cur->data = NULL;
    free (cur);
    cur = next_node; // move to the next node in line
  }
  free ((*(*(p_markov_chain))).database); // free linked list struct
  (*(*(p_markov_chain))).database = NULL;
  free (*(p_markov_chain)); // free markovchain
  *(p_markov_chain) = NULL;
}
