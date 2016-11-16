#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // strcmpy
#include <ctype.h>   // tolower

#define DEBUG      0
#define STR_NUM    32
#define STR_LENGTH 256
#define FILENAME   "input.txt"

// http://rextester.com/l/c_online_compiler_visual
// -- vc online compiler for compatibility checking

/// Convert <text> which contains <num> strings to lower register
int text_tolower (const char **text, size_t num, char **res, size_t *err);

/// input string from <fp>, it length could be greater than <size>
char *input_string (FILE* fp, size_t size, size_t *res);

/// input text from <fp>
char **input_text(FILE* fp, size_t *str_num_p, size_t *err);

/// print text
void print_text(char **text, size_t num);

/// free memory allocated for text
void free_text (char **text, size_t num);

int main(void)
{
  FILE *in = stdin;
#if DEBUG
  in = fopen (FILENAME, "r");
  if (!in)
    {
      printf ("[error]");
      return 0;
    }
#endif
  size_t err = 0;
  size_t num = STR_NUM;

  char **text = input_text (in, &num, &err);

  if (err)
    {
      free_text (text, num);
      printf ("[error]");
#if DEBUG
      fclose (in);
#endif
      return 0;
    }
#if DEBUG
  print_text (text, num);
#endif

  char **res = (char **) malloc (sizeof(char *) * num);
  if (!res)
    {
      free_text (text, num);
      printf ("[error]");
#if DEBUG
      fclose (in);
#endif
      return 0;
    }
  size_t counter = text_tolower ((const char**) text, num, res, &err);
  if (err)
    {
      free_text (text, num);
      free_text (res, num);
      printf ("[error]");
#if DEBUG
      fclose (in);
#endif
      return 0;
    }

  print_text (res, counter);
  free_text (text, num);
  free_text (res, counter);

#if DEBUG
  fclose(in);
#endif
  return 0;
}

int text_tolower(const char **text, size_t num, char **res, size_t *err)
{
  if (!text) return 0;

  size_t i, j;
  char *s, **realloc_res;

  size_t str_len, counter;
  for (i = 0, counter = 0; i < num; ++i)
    {
      s = (char *)text[i];
      if (!s) continue;
      str_len = strlen (s) + 1;
      res[counter] = (char *) malloc (sizeof(char) * str_len);
      if (!res[counter])
        {
          *err = (size_t) (1);
          return 0;
        }
      for (j = 0; j < str_len; ++j)
        {
          res[counter][j] = (char) tolower(s[j]);
        }
      ++counter;
    }

  if (counter != num)
    {
      realloc_res = (char **) realloc (res, sizeof(char *) * counter);
      if (!realloc_res)
        {
          *err = (size_t) (1);
          return 0;
        }
      res = realloc_res;
    }

  if (!res)
    {
      *err = (size_t) (1);
      return 0;
    }

  return counter;
}

char *input_string (FILE* fp, size_t size, size_t *res)
{
  char *str, *realloc_res;
  char ch;
  size_t len = 0;
  str = (char *) malloc (sizeof(char)*size);  // size is start size
  if (!str)
    {
      *res = (size_t) (-1);
      return str;
    }

  while (EOF != (ch = (char) fgetc (fp)) && ch != '\n')
    {
      str[len++] = ch;
      if (len == size)
        {
          realloc_res = (char *) realloc (str, sizeof(char)*(size += 16));
          if (!realloc_res)
            {
              *res = (size_t) (-1);
              return str;
            }
          str = realloc_res;
        }
    }
  str[len++] = '\0';
  realloc_res =  (char *) realloc(str, sizeof(char) * len);
  if (!realloc_res)
    {
      *res = (size_t) (-1);
      return str;
    }
  str = realloc_res;
  return str;
}

char **input_text(FILE* fp, size_t *str_num_p, size_t *err)
{
  char **txt, **realloc_res, *str;
  size_t len = 0;
  size_t num = *str_num_p;
  txt = (char **) realloc (NULL, sizeof(char *) * num); // num is start num
  if (!txt)
    {
      *err = (size_t) (1);
      return txt;
    }

  while (1)
    {
      str = input_string (fp, STR_LENGTH, err);
      if (*err || !*str)
        {
          if (str) free (str);
          break;
        }

      txt[len++] = str;
      if (len == num)
        {
          realloc_res = (char **) realloc (txt, sizeof(char *) * (num *= 2));
          if (!realloc_res)
            {
              *err = (size_t) (1);
              return txt;
            }
          txt = realloc_res;
        }
    }

  *str_num_p = len;
  *err = (size_t) (len == 0 ? 1 : 0);
  realloc_res = (char **) realloc (txt, sizeof (char *) * len);
  if (!realloc_res)
    {
      *err = (size_t) (1);
      return txt;
    }

  txt = realloc_res;
  return txt;
}

void free_text (char **text, size_t num)
{
  if (!text) return;
  unsigned int i = 0;
  while (i < num)
    free (text[i++]);
  free (text);
  return;
}

void print_text (char **text, size_t num)
{
  if (!text) return;
  unsigned int i = 0;
  while (i < num)
     printf ("%s\n", text[i++]);
  return;
}
