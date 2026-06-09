#include "soshell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

int main(void)
{
  char line[SHELL_MAX_LINE];
  char *argv[SHELL_MAX_ARGS];
  char prompt[SHELL_PROMPT_SIZE] = "SOSHELL> ";

  while (1)
  {
    char *input = readline(prompt);
    if (input == NULL)
    {
      putchar('\n');
      break;
    }

    if (input[0] != '\0')
      add_history(input);

    strncpy(line, input, sizeof(line) - 1);
    line[sizeof(line) - 1] = '\0';
    free(input);

    int argc = parse_line(line, argv, SHELL_MAX_ARGS);
    if (argc == 0)
      continue;

    if (run_builtin(prompt, sizeof(prompt), argv, argc))
      continue;

    execute_command(argv, argc);
  }

  return 0;
}