#include "soshell.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
  char line[SHELL_MAX_LINE];
  char *argv[SHELL_MAX_ARGS];
  char prompt[SHELL_PROMPT_SIZE] = "SOSHELL> ";

  while (1)
  {
    reap_background_processes();

    printf("%s", prompt);
    fflush(stdout);

    if (fgets(line, sizeof(line), stdin) == NULL)
    {
      putchar('\n');
      break;
    }

    size_t length = strlen(line);
    if (length > 0 && line[length - 1] == '\n')
      line[length - 1] = '\0';

    int argc = parse_line(line, argv, SHELL_MAX_ARGS);
    if (argc == 0)
      continue;

    if (run_builtin(prompt, sizeof(prompt), argv, argc))
      continue;

    execute_command(argv, argc);
  }

  return 0;
}