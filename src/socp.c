#include "soshell.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int write_all(int fd, const char *buffer, ssize_t count)
{
  ssize_t total = 0;

  while (total < count)
  {
    ssize_t written = write(fd, buffer + total, (size_t)(count - total));
    if (written < 0)
    {
      if (errno == EINTR)
        continue;

      return -1;
    }

    total += written;
  }

  return 0;
}

int shell_copy_file(const char *source_path, const char *dest_path)
{
  int source_fd = open(source_path, O_RDONLY);
  if (source_fd < 0)
  {
    perror(source_path);
    return -1;
  }

  int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (dest_fd < 0)
  {
    perror(dest_path);
    close(source_fd);
    return -1;
  }

  char buffer[COPY_BUFFER_SIZE];

  while (1)
  {
    ssize_t bytes_read = read(source_fd, buffer, sizeof(buffer));
    if (bytes_read == 0)
      break;

    if (bytes_read < 0)
    {
      if (errno == EINTR)
        continue;

      perror(source_path);
      close(source_fd);
      close(dest_fd);
      return -1;
    }

    if (write_all(dest_fd, buffer, bytes_read) != 0)
    {
      perror(dest_path);
      close(source_fd);
      close(dest_fd);
      return -1;
    }
  }

  if (close(source_fd) != 0)
    perror(source_path);

  if (close(dest_fd) != 0)
    perror(dest_path);

  return 0;
}