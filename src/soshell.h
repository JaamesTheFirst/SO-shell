#ifndef SOSHELL_H
#define SOSHELL_H

#include <stddef.h>

#define SHELL_MAX_LINE 1024
#define SHELL_MAX_ARGS 64
#define SHELL_PROMPT_SIZE 128
#define COPY_BUFFER_SIZE 4096

int parse_line(char *line, char **argv, int max_args);
int run_builtin(char *prompt, size_t prompt_size, char **argv, int argc);
void execute_command(char **argv, int argc);
void reap_background_processes(void);
int shell_copy_file(const char *source_path, const char *dest_path);
void shell_print_epsilon(void);
int shell_calc(const char *op, const char *left_text, const char *right_text);
int shell_bits(const char *op, const char *left_text, const char *right_text);
int shell_display_bit_ops(const char *left_text, const char *right_text);
int shell_is_jpeg(const char *path);
int shell_is_gif(const char *path);
int fd_is_valid(int fd);
int shell_open_file(const char *path);
int shell_close_fd(int fd);
int shell_read_fd(int fd, size_t byte_count);
void shell_file_info(void);

#endif