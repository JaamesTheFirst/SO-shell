#include "soshell.h"

#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static int is_home_shortcut(const char *path)
{
  return path == NULL || strcmp(path, "~") == 0 || strcmp(path, "$HOME") == 0;
}

static int parse_non_negative_long(const char *text, long *value)
{
  char *end = NULL;
  long parsed;

  errno = 0;
  parsed = strtol(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0' || parsed < 0)
  {
    fprintf(stderr, "invalid non-negative integer: %s\n", text);
    return 0;
  }

  *value = parsed;
  return 1;
}

static void print_shell_help(void)
{
  puts("Funcoes do soshell:");
  puts("  ?                   lista as funcoes do shell");
  puts("  sair                termina o shell");
  puts("  obterinfo           mostra informacao do shell");
  puts("  PS1=<texto>         muda o prompt");
  puts("  quemsoueu           mostra o utilizador atual");
  puts("  cd [diretorio]      muda de diretorio");
  puts("  socp origem destino copia um ficheiro");
  puts("  epsilon             mostra o epsilon de maquina");
  puts("  calc op a b         calculadora com doubles");
  puts("  bits op a b         operacoes binarias em unsigned short");
  puts("  displayBitOps a b   mostra operacoes e formatos de bits");
  puts("  isjpeg ficheiro     testa a assinatura JPEG");
  puts("  isgif ficheiro      testa a assinatura GIF");
  puts("  isValid fd          verifica se um descritor e valido");
  puts("  openfile ficheiro   abre um ficheiro em modo leitura");
  puts("  closefd fd          fecha um descritor aberto");
  puts("  read fd n           le n bytes de um descritor aberto");
  puts("  fileinfo            mostra info sobre descritores abertos");
  puts("  <, >, >>, 2>        redirecionamentos");
  puts("  comando1 | comando2 pipe simples");
  puts("  comando &           executa em background");
  puts("  outros comandos     executados com execvp()");
}

int run_builtin(char *prompt, size_t prompt_size, char **argv, int argc)
{
  if (strcmp(argv[0], "?") == 0)
  {
    print_shell_help();
    return 1;
  }

  if (strcmp(argv[0], "sair") == 0)
    exit(0);

  if (strcmp(argv[0], "obterinfo") == 0)
  {
    printf("SoShell 2026 versao 1.2\n");
    return 1;
  }

  if (strncmp(argv[0], "PS1=", 4) == 0)
  {
    snprintf(prompt, prompt_size, "%s", argv[0] + 4);
    return 1;
  }

  if (strcmp(argv[0], "quemsoueu") == 0)
  {
    uid_t uid = getuid();
    struct passwd *user = getpwuid(uid);

    if (user == NULL)
      perror("getpwuid");
    else
      printf("Sou utilizador: %s\n", user->pw_name);

    return 1;
  }

  if (strcmp(argv[0], "cd") == 0)
  {
    const char *target = argc > 1 ? argv[1] : NULL;

    if (is_home_shortcut(target))
    {
      target = getenv("HOME");
      if (target == NULL)
      {
        fprintf(stderr, "cd: HOME is not set\n");
        return 1;
      }
    }

    if (chdir(target) != 0)
      perror(target);

    return 1;
  }

  if (strcmp(argv[0], "socp") == 0)
  {
    if (argc != 3)
    {
      fprintf(stderr, "usage: socp source destination\n");
      return 1;
    }

    shell_copy_file(argv[1], argv[2]);
    return 1;
  }

  if (strcmp(argv[0], "epsilon") == 0)
  {
    shell_print_epsilon();
    return 1;
  }

  if (strcmp(argv[0], "calc") == 0)
  {
    if (argc != 4)
    {
      fprintf(stderr, "usage: calc <op> <value1> <value2>\n");
      return 1;
    }

    shell_calc(argv[1], argv[2], argv[3]);
    return 1;
  }

  if (strcmp(argv[0], "bits") == 0)
  {
    if (argc != 4)
    {
      fprintf(stderr, "usage: bits <op> <value1> <value2>\n");
      return 1;
    }

    shell_bits(argv[1], argv[2], argv[3]);
    return 1;
  }

  if (strcmp(argv[0], "displayBitOps") == 0)
  {
    if (argc != 3)
    {
      fprintf(stderr, "usage: displayBitOps <value1> <value2>\n");
      return 1;
    }

    shell_display_bit_ops(argv[1], argv[2]);
    return 1;
  }

  if (strcmp(argv[0], "isjpeg") == 0)
  {
    int result;

    if (argc != 2)
    {
      fprintf(stderr, "usage: isjpeg <file>\n");
      return 1;
    }

    result = shell_is_jpeg(argv[1]);
    if (result >= 0)
      printf("%s %s JPEG\n", argv[1], result ? "is" : "is not");

    return 1;
  }

  if (strcmp(argv[0], "isgif") == 0)
  {
    int result;

    if (argc != 2)
    {
      fprintf(stderr, "usage: isgif <file>\n");
      return 1;
    }

    result = shell_is_gif(argv[1]);
    if (result >= 0)
      printf("%s %s GIF\n", argv[1], result ? "is" : "is not");

    return 1;
  }

  if (strcmp(argv[0], "isValid") == 0)
  {
    long fd_value;

    if (argc != 2)
    {
      fprintf(stderr, "usage: isValid <fd>\n");
      return 1;
    }

    if (parse_non_negative_long(argv[1], &fd_value))
      printf("%ld is %svalid\n", fd_value, fd_is_valid((int)fd_value) ? "" : "not ");

    return 1;
  }

  if (strcmp(argv[0], "openfile") == 0)
  {
    if (argc != 2)
    {
      fprintf(stderr, "usage: openfile <file>\n");
      return 1;
    }

    shell_open_file(argv[1]);
    return 1;
  }

  if (strcmp(argv[0], "closefd") == 0)
  {
    long fd_value;

    if (argc != 2)
    {
      fprintf(stderr, "usage: closefd <fd>\n");
      return 1;
    }

    if (parse_non_negative_long(argv[1], &fd_value))
      shell_close_fd((int)fd_value);

    return 1;
  }

  if (strcmp(argv[0], "read") == 0)
  {
    long fd_value;
    long byte_count;

    if (argc != 3)
    {
      fprintf(stderr, "usage: read <fd> <bytes>\n");
      return 1;
    }

    if (!parse_non_negative_long(argv[1], &fd_value) ||
        !parse_non_negative_long(argv[2], &byte_count))
      return 1;

    if ((unsigned long)byte_count > (unsigned long)SIZE_MAX)
    {
      fprintf(stderr, "byte count too large: %ld\n", byte_count);
      return 1;
    }

    shell_read_fd((int)fd_value, (size_t)byte_count);
    return 1;
  }

  if (strcmp(argv[0], "fileinfo") == 0)
  {
    shell_file_info();
    return 1;
  }

  return 0;
}