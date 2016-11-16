#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <search.h>

#define STACK_MAX        100
#define MAX_NUM_LEN        9
#define SET_START_LEN     16
#define ALLOWED_NUMB       8
#define SET_ALLOWED_NUMB  14
#define DIGIT_NUMB        10
#define file_name         "input.txt"
#define DEBUG             0

// Set struct specification ----------------------------------------------------
struct set
{
  int    *arr; ///< unsorted array of distinct numbers
  size_t size;
};
typedef struct set set;

// Set struct methods:

// Init empty set (arr == NULL, size == 0).
void init_set (set *A);

// Input set from <in> as distinct numbers [el1, el2, el2, ...].
// Starting len in <start_len>. Could be resized if needed.
set *input_set (FILE *in, size_t start_len, size_t *err);

// Free set memory and reinit it as empty set.
void free_set (set *A);

// Print set as [el1,el2,el3,...] in stdin.
void print_set (set *A);

// Set operations:
set *set_op_unite     (set *A, set *B, size_t *err);
set *set_op_intersect (set *A, set *B, size_t *err);
set *set_op_substract (set *A, set *B, size_t *err);
int merge_arr_op_unite (int *arr, int size, int *res, int res_initial_size);

// Stack of chars specification ------------------------------------------------
struct stack_char
{
  char data[STACK_MAX];
  int size;
};
typedef struct stack_char stack_char;

// Stack of chars methods:
void stack_char_init (stack_char *S);
char stack_char_top (stack_char *S, size_t *err);
void stack_char_push (stack_char *S, char d, size_t *err);
char stack_char_pop (stack_char *S, size_t *err);

// Stack of sets specification -------------------------------------------------
struct stack_set
{
  set data[STACK_MAX];
  int size;
};
typedef struct stack_set stack_set;

/// stack set methods
void stack_set_init (stack_set *S);
void stack_set_free (stack_set *S);
set *stack_set_top(stack_set *S, size_t *err);
void stack_set_push (stack_set *S, set *st, size_t *err);
set *stack_set_pop(stack_set *S, size_t *err);

const char ALLOWED[ALLOWED_NUMB] = {'U', '^', '\\', '(', ')', '[', ' ', '\n'};

const char SET_ALLOWED[SET_ALLOWED_NUMB] = {']', '0', '1', '2', '3', '4', '5',
                                            '6', '7', '8', '9', ',', ' ', '\n'};

int single_set_input_not_allowed (char ch);
int input_not_allowed (char ch);
int is_func (char ch);

// Diekstra 2-stack algoritm methods
void pop_functions (stack_set *operands, stack_char *functions, size_t *err);
int can_pop (char op1, stack_char *functions, size_t *err);
int get_priority (char op);
int compareints (const void * a, const void * b);

int main(void)
{
  FILE *in = stdin;
#if DEBUG
  in = fopen (file_name, "r");
  if (!in)
    {
      printf ("Canot open file <%s>.\n", file_name);
      return 0;
    }
#endif

  stack_char functions;
  stack_char_init (&functions);
  stack_set operands;
  stack_set_init (&operands);

  size_t err = 0;
  size_t brace_is_opened = 0;
  size_t func_is_expected = 0;
  char ch;

  set *p_res_st = NULL;

  while (EOF != (ch = fgetc (in)) && (ch != '\n'))
    {
      if (!err && input_not_allowed (ch))
        {
          err = (size_t) (1);
          if (DEBUG) printf ("Error! Symbol <%c> is not expected in main input. \n", ch);
        }

      if (err)
        {
          stack_set_free (&operands);
          stack_char_init (&functions);
          if (!DEBUG) printf ("[error]\n");
          break;
        }

      if (ch == ' ')
        continue;

      if (ch == '[')
        {
          if (func_is_expected == 1)
            {
              err = (size_t) (1);
              if (DEBUG)
                printf ("Error! Symbol <%c> is not expected in main input. \n", ch);
            }
          else
            {
              p_res_st = input_set (in, SET_START_LEN, &err);
              stack_set_push (&operands, p_res_st, &err);
              free (p_res_st);
              func_is_expected = 1;
            }
        }
      else
        {
          if (ch == ')')
            {
              if (brace_is_opened < 1)
                {
                  err = (size_t) (1);
                  if (DEBUG)
                    printf ("Error! Unallowed char sequence in main input. \n");
                }
              else
                {
                  while (functions.size > 0 && stack_char_top (&functions, &err) != '(')
                    pop_functions (&operands, &functions, &err);
                  stack_char_pop (&functions, &err); // delete '('
                }
              brace_is_opened--;
            }
          else
            {
              if (is_func (ch) && func_is_expected == 0)
                {
                  err = (size_t) (1);
                  if (DEBUG)
                    printf ("Error! Symbol <%c> is not expected in main input. \n", ch);
                }
              if (ch == '(')
                brace_is_opened += 1;

              while (can_pop (ch, &functions, &err))
                pop_functions (&operands, &functions, &err);

              stack_char_push (&functions, ch, &err);
              func_is_expected = 0;
            }
        }
    }

  if (brace_is_opened != 0)
    {
      if (DEBUG)
        printf ("Error! Not all braces are closed. \n");
      else
        printf ("[error]\n");

      stack_set_free (&operands);
      stack_char_init (&functions);
    }

  while (functions.size > 0)
    {
      pop_functions (&operands, &functions, &err);
    }

  if (err)
    {
      printf ("[error]\n");
      stack_set_free (&operands);
      stack_char_init (&functions);
      err = 0;
    }

  if (operands.size > 0)
    {
      p_res_st = stack_set_pop (&operands, &err);
      qsort (p_res_st->arr, p_res_st->size, sizeof (int), compareints);
      print_set (p_res_st);
      free_set (p_res_st);
    }

  if (err)
    {
      printf ("[error]\n");
      stack_set_free (&operands);
      stack_char_init (&functions);
    }

#if DEBUG
  fclose (in);
#endif

  return 0;
}

void init_set (set *A)
{
  if (!A) return;
  A->arr = NULL;
  A->size = 0;
}

void stack_char_init (stack_char *S)
{
  if (!S) return;
  S->size = 0;
}

char stack_char_top (stack_char *S, size_t *err)
{
  if (!S) return '\0';
  if (S->size == 0)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Error! Char stack is empty. \n");
      return -1;
    }
  return S->data[S->size-1];
}

void stack_char_push (stack_char *S, char d, size_t *err)
{
  if (!S) return;
  if (S->size < STACK_MAX)
    S->data[S->size++] = d;
  else
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Error! Char stack is full. \n");
    }
}

char stack_char_pop (stack_char *S, size_t *err)
{
  if (!S) return '\0';
  if (S->size == 0)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Error! Char stack is empty. \n");
      return '\0';
    }
  else
    {
      S->size--;
      return S->data[S->size];
    }
}

void stack_set_init (stack_set *S)
{
  if (!S) return;
  size_t i = 0;
  while (i < STACK_MAX)
    init_set (&S->data[i++]);
  S->size = 0;
}

void stack_set_free (stack_set *S)
{
  if (!S) return;
  size_t err; // ignored
  while (S->size > 0)
    {
      free_set (stack_set_pop (S, &err));
    }
}

set *stack_set_top (stack_set *S, size_t *err)
{
  if (!S) return '\0';
  if (S->size == 0)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Error! Set stack is empty.\n");
      return NULL;
    }
  return &S->data[S->size-1];
}

void stack_set_push (stack_set *S, set *st, size_t *err)
{
  if (!S || !st) return;
  if (S->size < STACK_MAX)
    {
      S->data[S->size++] = *st;
    }
  else
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Error! Set stack is full.\n");
    }
}

set *stack_set_pop (stack_set *S, size_t *err)
{
  if (!S) return NULL;
  if (S->size == 0)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Error! Stack is empty. \n");
      return NULL;
    }
  else
    {
      S->size--;
      return &S->data[S->size];
    }
}

set *input_set (FILE *in, size_t start_len, size_t *err)
{
  char ch;
  char num[MAX_NUM_LEN + 1]; // + '\0' for atoi usage
  int  num_len = 0;
  int *alloc_res;

  set *p_res = (set *) malloc (sizeof(set));
  if (!p_res)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Memory allocation error in unite_op.\n");
      return NULL;
    }
  init_set (p_res);

  alloc_res = (int *) malloc (sizeof (int) * start_len);
  if (!alloc_res)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Memory allocation error in input_set() \n");
      return NULL;
    }
  p_res->arr = alloc_res;

  size_t len = start_len;
  while (EOF != (ch = fgetc (in)))
    {
      if (single_set_input_not_allowed (ch))
        {
          *err = (size_t) (1);
          if (DEBUG) printf ("Error! Unallowed char <%c> in input_set(). \n", ch);
          free_set (p_res);
          return NULL;

        }
      if (isdigit (ch))
        {
          if (num_len < MAX_NUM_LEN)
            num[num_len++] = (int) ch;
          else
            {
              *err = (size_t) (1);
              if (DEBUG) printf ("Error! Number in set has more digits than %d", (int) MAX_NUM_LEN);
              free_set (p_res);
              return NULL;
            }
        }
      else if (num_len != 0)
        {
          num[num_len] = '\0';
          int numint = strtol (num, NULL, 10);
          if (p_res->size == len)
            {
              // p_res->arr is full. need reallocation.
              len *= 2;
              alloc_res = (int *) realloc (p_res->arr, sizeof (int) * len);
              if (!alloc_res)
                {
                  *err = (size_t) (1);
                  if (DEBUG) printf ("Memory reallocation error in input_set() \n");
                  free_set (p_res);
                  return NULL;
                }
              p_res->arr = alloc_res;
            }

          int *p_item = (int *) lfind (&numint, p_res->arr, &p_res->size, sizeof (int), compareints);
          if (p_item == NULL) // elem is not in arr
            {
              p_res->arr[p_res->size++] = numint;
            }
          num_len = 0;
        }
      if (ch == ']')
        break;
    }

  if (p_res->size == 0)
    {
      free_set (p_res);
      return p_res;
    }

  alloc_res = (int *) realloc (p_res->arr, sizeof (int) * p_res->size);
  if (!alloc_res)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Memory reallocation error in input_set() \n");
      free_set (p_res);
      return NULL;
    }
  p_res->arr = alloc_res;
  return p_res;
}

void pop_functions (stack_set *operands, stack_char *functions, size_t *err)
{
  set *B = stack_set_pop (operands, err);
  set *A = stack_set_pop (operands, err);
  set *C = NULL;
  switch (stack_char_pop (functions, err))
    {
      case 'U':
        C = set_op_unite (A, B, err);
        stack_set_push (operands, C, err);
        break;
      case '\\':
        C = set_op_substract (A, B, err);
        stack_set_push (operands, C, err);
        break;
      case '^':
        C = set_op_intersect (A, B, err);
        stack_set_push (operands, C, err);
        break;
      default:
        free_set (A);
        free_set (B);
        break;
    }
  if (A == NULL || B == NULL)
    {
      free_set (A);
      free_set (B);
    }
  else
    free (C);
}

int can_pop (char op1, stack_char *functions, size_t *err)
{
  if (functions->size == 0) return 0;
  int p1 = get_priority (op1);
  int p2 = get_priority (stack_char_top (functions, err));
  if (*err) return 0;
  return p1 >= 0 && p2 >= 0 && p1 >= p2;
}

int get_priority (char op)
{
  switch (op)
    {
      case '(':
        return -1;
      case '^':
        return 1;
      case 'U': case '\\':
        return 2;
      default:
        // error
        return 0;
    }
}

int compareints (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int merge_arr_op_unite (int *arr, int size, int *res, int res_initial_size)
{
  size_t effective_size = res_initial_size;
  int i, curr_elem, *p_item;
  for (i = 0; i < size; i++)
    {
      curr_elem = arr[i];
      p_item = (int *) lfind (&curr_elem, res, &effective_size, sizeof (int), compareints);
      if (p_item == NULL) // elem is not in arr
        {
          res[effective_size++] = curr_elem;
        }
    }
  return effective_size;
}

set *set_op_unite (set *A, set *B, size_t *err)
{
  if (A == NULL && B == NULL)
    return NULL;
  else if (A == NULL)
    return B;
  else if (B == NULL)
    return A;

  int set_size = A->size + B->size;
  int *set_arr = (int *) malloc (sizeof(int) * set_size);
  if (!set_arr)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Memory allocation error in unite_op.\n");
      free_set (A);
      free_set (B);
      return NULL;
    }
  int *realloc_res;
  int effective_size = merge_arr_op_unite (A->arr, A->size, set_arr, 0);
  effective_size = merge_arr_op_unite (B->arr, B->size, set_arr, effective_size);

  if (set_size != effective_size)
    {
      realloc_res = (int *) realloc (set_arr, sizeof(int) * effective_size);
      if (!realloc_res)
        {
          *err = (size_t) (1);
          if (DEBUG) printf ("Memory allocation error in unite_op.\n");
          free_set (A);
          free_set (B);
          free (set_arr);
          return NULL;
        }
      set_arr = realloc_res;
    }

  free_set (A);
  free_set (B);

  set *res = (set *) malloc (sizeof(set));
  if (!res)
    {
      if (DEBUG) printf ("Memory allocation error in unite_op.\n");
      *err = (size_t) (1);
      free (set_arr);
      return NULL;
    }
  init_set (res);
  res->arr = set_arr;
  res->size = effective_size;
  return res;
}

set *set_op_intersect (set *A, set *B, size_t *err)
{
  if (A == NULL || B == NULL)
    return NULL;

  int set_size = (A->size > B->size ? A->size : B->size); // max size
  int *set_arr = (int *) malloc (sizeof(int) * set_size);
  if (!set_arr)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Memory allocation error in intersect_op.\n");
      free_set (A);
      free_set (B);
      return NULL;
    }

  int effective_size = 0;
  int curr_elem, *p_item, *realloc_res;
  size_t i;
  for (i = 0; i < A->size; i++)
    {
      curr_elem = A->arr[i];
      p_item = (int *) lfind (&curr_elem, B->arr, &B->size, sizeof (int), compareints);
      if (p_item != NULL) // elem is in B
        {
          set_arr[effective_size++] = curr_elem;
        }
    }

  if (set_size != effective_size && effective_size > 0)
    {
      realloc_res = (int *) realloc (set_arr, sizeof(int) * effective_size);
      if (!realloc_res)
        {
          *err = (size_t) (1);
          if (DEBUG) printf ("Memory allocation error in intersect_op.\n");
          free_set (A);
          free_set (B);
          free (set_arr);
          return NULL;
        }
      set_arr = realloc_res;
    }
  else
    {
      set_size = 0;
    }

  free_set (A);
  free_set (B);

  set *res = (set *) malloc (sizeof(set));
  if (!res)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Memory allocation error in intersect_op.\n");
      free (set_arr);
      return NULL;
    }
  init_set (res);
  res->arr = set_arr;
  res->size = effective_size;
  return res;
}

set *set_op_substract (set *A, set *B, size_t *err)
{
  if (A == NULL && B == NULL)
    return NULL;
  else if (A == NULL)
    return NULL;
  else if (B == NULL)
    return A;

  int set_size = A->size;
  int *set_arr = (int *) malloc (sizeof(int) * set_size);
  if (!set_arr)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Memory allocation error in unite_op.\n");
      free_set (A);
      free_set (B);
      return NULL;
    }

  int effective_size = 0;
  int curr_elem, *p_item, *realloc_res;
  size_t i;
  for (i = 0; i < A->size; i++)
    {
      curr_elem = A->arr[i];
      p_item = (int *) lfind (&curr_elem, B->arr, (size_t *)&B->size, sizeof (int), compareints);
      if (p_item == NULL) // elem is not in B
        {
          set_arr[effective_size++] = curr_elem;
        }
    }

  if (set_size != effective_size && effective_size > 0)
    {
      realloc_res = (int *) realloc (set_arr, sizeof(int) * effective_size);
      if (!realloc_res)
        {
          *err = (size_t) (1);
          free_set (A);
          free_set (B);
          free (set_arr);
          return NULL;
        }
      set_arr = realloc_res;
    }
  else
    {
      set_size = 0;
    }

  free_set (A);
  free_set (B);

  set *res = (set *) malloc (sizeof(set));
  if (!res)
    {
      *err = (size_t) (1);
      if (DEBUG) printf ("Memory allocation error in unite_op.\n");
      free (set_arr);
      return NULL;
    }
  init_set (res);
  res->arr = set_arr;
  res->size = effective_size;
  return res;
}

void free_set (set *A)
{
  if (!A)
    return;
  if (A->arr)
    free (A->arr);
  init_set (A);
}

void print_set (set *A)
{
  if (!A) return;
  size_t i;
  printf ("[");
  for (i = 0; i < A->size; i++)
    {
      printf ("%d", A->arr[i]);
      if (i < A->size - 1)
        printf (",");
    }
  printf ("]\n");
}

int single_set_input_not_allowed (char ch)
{
  size_t i;
  for (i = 0; i < SET_ALLOWED_NUMB; i++)
    {
      if (SET_ALLOWED[i] == ch)
        return 0;
    }
  return 1;
}

int input_not_allowed (char ch)
{
  size_t i;
  for (i = 0; i < ALLOWED_NUMB; i++)
    {
      if (ALLOWED[i] == ch)
        return 0;
    }
  return 1;
}

int is_func (char ch)
{
  return (ch == 'U' || ch == '\\' || ch == '^');
}
