#include "soshell.h"

#include <ctype.h>
#include <stdio.h>

int parse_line(char *line, char **argv, int max_args)
{
  int argc = 0;
  char *cursor = line;

  while (*cursor != '\0')
  {
    while (isspace((unsigned char)*cursor))
    {
      *cursor = '\0';
      cursor++;
    }

    if (*cursor == '\0')
      break;

    if (argc == max_args - 1)
    {
      fprintf(stderr, "too many arguments\n");
      break;
    }

    argv[argc++] = cursor;

    while (*cursor != '\0' && !isspace((unsigned char)*cursor))
      cursor++;
  }

  argv[argc] = NULL;
  return argc;
}