#include "soshell.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void close_fd(int fd)
{
  if (fd >= 0)
    close(fd);
}

static void wait_for_child(pid_t pid)
{
  while (waitpid(pid, NULL, 0) < 0)
  {
    if (errno != EINTR)
    {
      perror("waitpid");
      break;
    }
  }
}

static int find_pipe_index(char **argv, int argc)
{
  int index;

  for (index = 0; index < argc; index++)
  {
    if (strcmp(argv[index], "|") == 0)
      return index;
  }

  return -1;
}

static int open_output_file(const char *path, int append)
{
  int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
  return open(path, flags, 0644);
}

static void redirect_fd(int source_fd, int target_fd)
{
  if (source_fd < 0)
    return;

  if (dup2(source_fd, target_fd) < 0)
  {
    perror("dup2");
    _exit(1);
  }

  close_fd(source_fd);
}

static void exec_segment(char **argv, int argc)
{
  char *exec_argv[SHELL_MAX_ARGS];
  int exec_argc = 0;
  int input_fd = -1;
  int output_fd = -1;
  int error_fd = -1;

  int index;
  for (index = 0; index < argc; index++)
  {
    if (strcmp(argv[index], "<") == 0 || strcmp(argv[index], ">") == 0 ||
        strcmp(argv[index], ">>") == 0 || strcmp(argv[index], "2>") == 0)
    {
      if (index + 1 >= argc)
      {
        fprintf(stderr, "syntax error near '%s'\n", argv[index]);
        _exit(1);
      }

      if (strcmp(argv[index], "<") == 0)
      {
        close_fd(input_fd);
        input_fd = open(argv[index + 1], O_RDONLY);
        if (input_fd < 0)
        {
          perror(argv[index + 1]);
          _exit(1);
        }
      }
      else if (strcmp(argv[index], ">") == 0)
      {
        close_fd(output_fd);
        output_fd = open_output_file(argv[index + 1], 0);
        if (output_fd < 0)
        {
          perror(argv[index + 1]);
          _exit(1);
        }
      }
      else if (strcmp(argv[index], ">>") == 0)
      {
        close_fd(output_fd);
        output_fd = open_output_file(argv[index + 1], 1);
        if (output_fd < 0)
        {
          perror(argv[index + 1]);
          _exit(1);
        }
      }
      else
      {
        close_fd(error_fd);
        error_fd = open_output_file(argv[index + 1], 0);
        if (error_fd < 0)
        {
          perror(argv[index + 1]);
          _exit(1);
        }
      }

      index++;
      continue;
    }

    if (exec_argc == SHELL_MAX_ARGS - 1)
    {
      fprintf(stderr, "too many command arguments\n");
      _exit(1);
    }

    exec_argv[exec_argc++] = argv[index];
  }

  exec_argv[exec_argc] = NULL;

  if (exec_argc == 0)
  {
    fprintf(stderr, "empty command\n");
    _exit(1);
  }

  redirect_fd(input_fd, STDIN_FILENO);
  redirect_fd(output_fd, STDOUT_FILENO);
  redirect_fd(error_fd, STDERR_FILENO);

  execvp(exec_argv[0], exec_argv);
  perror(exec_argv[0]);
  _exit(1);
}

static void start_simple_command(char **argv, int argc, int background)
{
  pid_t pid = fork();

  if (pid < 0)
  {
    perror("fork");
    return;
  }

  if (pid == 0)
    exec_segment(argv, argc);

  if (background)
    printf("[background pid %ld]\n", (long)pid);
  else
    wait_for_child(pid);
}

static void start_pipeline(char **argv, int argc, int pipe_index, int background)
{
  int pipe_fds[2];
  if (pipe(pipe_fds) < 0)
  {
    perror("pipe");
    return;
  }

  pid_t left_pid = fork();
  if (left_pid < 0)
  {
    perror("fork");
    close_fd(pipe_fds[0]);
    close_fd(pipe_fds[1]);
    return;
  }

  if (left_pid == 0)
  {
    close_fd(pipe_fds[0]);
    redirect_fd(pipe_fds[1], STDOUT_FILENO);
    exec_segment(argv, pipe_index);
  }

  pid_t right_pid = fork();
  if (right_pid < 0)
  {
    perror("fork");
    close_fd(pipe_fds[0]);
    close_fd(pipe_fds[1]);
    wait_for_child(left_pid);
    return;
  }

  if (right_pid == 0)
  {
    close_fd(pipe_fds[1]);
    redirect_fd(pipe_fds[0], STDIN_FILENO);
    exec_segment(argv + pipe_index + 1, argc - pipe_index - 1);
  }

  close_fd(pipe_fds[0]);
  close_fd(pipe_fds[1]);

  if (background)
  {
    printf("[background pids %ld %ld]\n", (long)left_pid, (long)right_pid);
    return;
  }

  wait_for_child(left_pid);
  wait_for_child(right_pid);
}

void execute_command(char **argv, int argc)
{
  if (argc == 0)
    return;

  int background = 0;
  if (strcmp(argv[argc - 1], "&") == 0)
  {
    background = 1;
    argc--;
    if (argc == 0)
      return;
  }

  int pipe_index = find_pipe_index(argv, argc);
  if (pipe_index >= 0)
  {
    if (pipe_index == 0 || pipe_index == argc - 1)
    {
      fprintf(stderr, "invalid pipe usage\n");
      return;
    }

    start_pipeline(argv, argc, pipe_index, background);
    return;
  }

  start_simple_command(argv, argc, background);
}

void reap_background_processes(void)
{
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}