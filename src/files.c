#define _POSIX_C_SOURCE 200809L

#include "soshell.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

static int read_prefix(int fd, unsigned char *buffer, size_t buffer_size)
{
  size_t total = 0;

  while (total < buffer_size)
  {
    ssize_t bytes_read = read(fd, buffer + total, buffer_size - total);
    if (bytes_read < 0)
    {
      if (errno == EINTR)
        continue;

      return -1;
    }

    if (bytes_read == 0)
      break;

    total += (size_t)bytes_read;
  }

  return (int)total;
}

static int detect_jpeg_fd(int fd)
{
  unsigned char magic[4];
  int bytes_read = read_prefix(fd, magic, sizeof(magic));

  if (lseek(fd, 0, SEEK_SET) < 0)
    perror("lseek");

  if (bytes_read < 0)
    return -1;

  if (bytes_read != 4)
    return 0;

  return magic[0] == 0xff && magic[1] == 0xd8 && magic[2] == 0xff &&
         (magic[3] == 0xe0 || magic[3] == 0xe1 || magic[3] == 0xe2 || magic[3] == 0xe8);
}

static int detect_gif_fd(int fd)
{
  unsigned char magic[6];
  int bytes_read = read_prefix(fd, magic, sizeof(magic));

  if (lseek(fd, 0, SEEK_SET) < 0)
    perror("lseek");

  if (bytes_read < 0)
    return -1;

  if (bytes_read != 6)
    return 0;

  return memcmp(magic, "GIF87a", 6) == 0 || memcmp(magic, "GIF89a", 6) == 0;
}

int fd_is_valid(int fd)
{
  if (fd < 0)
    return 0;

  errno = 0;
  return !(fcntl(fd, F_GETFD) == -1 && errno == EBADF);
}

int shell_is_jpeg(const char *path)
{
  int fd = open(path, O_RDONLY);
  int result;

  if (fd < 0)
  {
    perror(path);
    return -1;
  }

  result = detect_jpeg_fd(fd);
  if (result < 0)
    perror(path);

  if (close(fd) != 0)
    perror(path);

  return result;
}

int shell_is_gif(const char *path)
{
  int fd = open(path, O_RDONLY);
  int result;

  if (fd < 0)
  {
    perror(path);
    return -1;
  }

  result = detect_gif_fd(fd);
  if (result < 0)
    perror(path);

  if (close(fd) != 0)
    perror(path);

  return result;
}

int shell_open_file(const char *path)
{
  int fd = open(path, O_RDONLY);

  if (fd < 0)
  {
    perror(path);
    return -1;
  }

  printf("Opened %s ok with fd %d\n", path, fd);
  return fd;
}

int shell_close_fd(int fd)
{
  if (close(fd) != 0)
  {
    perror("closefd");
    return -1;
  }

  printf("%d closed ok\n", fd);
  return 0;
}

int shell_read_fd(int fd, size_t byte_count)
{
  unsigned char *buffer;
  ssize_t bytes_read;
  size_t index;

  if (!fd_is_valid(fd))
  {
    fprintf(stderr, "invalid file descriptor: %d\n", fd);
    return -1;
  }

  buffer = malloc(byte_count == 0 ? 1 : byte_count);
  if (buffer == NULL)
  {
    perror("malloc");
    return -1;
  }

  bytes_read = read(fd, buffer, byte_count);
  if (bytes_read < 0)
  {
    perror("read");
    free(buffer);
    return -1;
  }

  printf("ASCII: ");
  for (index = 0; index < (size_t)bytes_read; index++)
  {
    unsigned char byte = buffer[index];
    putchar(isprint(byte) || byte == ' ' ? byte : '.');
  }
  putchar('\n');

  puts("--hex values--");
  for (index = 0; index < (size_t)bytes_read; index++)
    printf("%02x ", buffer[index]);
  putchar('\n');

  free(buffer);
  return 0;
}

void shell_file_info(void)
{
  int stdout_fd = fileno(stdout);
  struct rlimit limits;
  rlim_t scan_limit;
  rlim_t fd;
  size_t open_count = 0;

  if (stdout != NULL && stdout_fd >= 0 && fd_is_valid(stdout_fd))
    printf("STDOUT is open : file number %d\n", stdout_fd);
  else if (stdout != NULL)
    printf("STDOUT uses file number %d but it is currently closed\n", stdout_fd);
  else
    puts("STDOUT is closed");

  if (getrlimit(RLIMIT_NOFILE, &limits) != 0)
  {
    perror("getrlimit");
    return;
  }

  if (limits.rlim_cur == RLIM_INFINITY)
  {
    puts("Current Process has no fixed file limit");
    scan_limit = 1024;
  }
  else
  {
    printf("Current Process has a %lu File Limit\n", (unsigned long)limits.rlim_cur);
    scan_limit = limits.rlim_cur;
  }

  printf("Open file descriptors:");
  for (fd = 0; fd < scan_limit; fd++)
  {
    if (fd_is_valid((int)fd))
    {
      printf(" %lu", (unsigned long)fd);
      open_count++;
    }
  }
  putchar('\n');

  printf("Process has %zu open files\n", open_count);
}