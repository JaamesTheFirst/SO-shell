#include "soshell.h"

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_double_arg(const char *text, double *value)
{
  char *end = NULL;
  double parsed;

  errno = 0;
  parsed = strtod(text, &end);
  if (errno != 0 || end == text || *end != '\0')
  {
    fprintf(stderr, "invalid number: %s\n", text);
    return 0;
  }

  *value = parsed;
  return 1;
}

static int parse_unsigned_short_arg(const char *text, unsigned short *value)
{
  char *end = NULL;
  unsigned long parsed;

  errno = 0;
  parsed = strtoul(text, &end, 0);
  if (errno != 0 || end == text || *end != '\0' || parsed > USHRT_MAX)
  {
    fprintf(stderr, "invalid unsigned short: %s\n", text);
    return 0;
  }

  *value = (unsigned short)parsed;
  return 1;
}

static void print_bits(unsigned short value)
{
  unsigned short mask = 0x8000;

  while (mask > 0)
  {
    putchar((value & mask) == 0 ? '0' : '1');
    mask >>= 1;
  }
}

void shell_print_epsilon(void)
{
  float float_epsilon = 1.0f;
  double double_epsilon = 1.0;

  while ((1.0f + float_epsilon / 2.0f) != 1.0f)
    float_epsilon /= 2.0f;

  while ((1.0 + double_epsilon / 2.0) != 1.0)
    double_epsilon /= 2.0;

  printf("Biblioteca: float=%10e double=%10e\n", FLT_EPSILON, DBL_EPSILON);
  printf("Calculado : float=%10e double=%10e\n", float_epsilon, double_epsilon);
}

int shell_calc(const char *op, const char *left_text, const char *right_text)
{
  double left;
  double right;
  double result;

  if (!parse_double_arg(left_text, &left) || !parse_double_arg(right_text, &right))
    return -1;

  if (strcmp(op, "+") == 0)
    result = left + right;
  else if (strcmp(op, "-") == 0)
    result = left - right;
  else if (strcmp(op, "*") == 0)
    result = left * right;
  else if (strcmp(op, "/") == 0)
  {
    if (fabs(right) <= DBL_EPSILON)
    {
      fprintf(stderr, "division by zero\n");
      return -1;
    }

    result = left / right;
  }
  else if (strcmp(op, "^") == 0)
    result = pow(left, right);
  else
  {
    fprintf(stderr, "invalid operator: %s\n", op);
    return -1;
  }

  printf("Resultado calc %.3f %s %.3f = %.3f\n", left, op, right, result);
  return 0;
}

int shell_bits(const char *op, const char *left_text, const char *right_text)
{
  unsigned short left;
  unsigned short right;
  unsigned short result;

  if (!parse_unsigned_short_arg(left_text, &left) ||
      !parse_unsigned_short_arg(right_text, &right))
    return -1;

  if (strcmp(op, "&") == 0)
    result = (unsigned short)(left & right);
  else if (strcmp(op, "|") == 0)
    result = (unsigned short)(left | right);
  else if (strcmp(op, "^") == 0)
    result = (unsigned short)(left ^ right);
  else
  {
    fprintf(stderr, "invalid bit operator: %s\n", op);
    return -1;
  }

  printf("Resultado bits %hu %s %hu = %hu\n", left, op, right, result);
  return 0;
}

int shell_display_bit_ops(const char *left_text, const char *right_text)
{
  unsigned short left;
  unsigned short right;
  unsigned short and_result;
  unsigned short or_result;
  unsigned short xor_result;
  unsigned short clear_result;

  if (!parse_unsigned_short_arg(left_text, &left) ||
      !parse_unsigned_short_arg(right_text, &right))
    return -1;

  and_result = (unsigned short)(left & right);
  or_result = (unsigned short)(left | right);
  xor_result = (unsigned short)(left ^ right);
  clear_result = (unsigned short)(left & (unsigned short)~right);

  puts("Valor 1:");
  printf("  binario: ");
  print_bits(left);
  printf("\n  decimal: %hu\n  octal: %ho\n  hexadecimal: 0x%04hx\n",
         left, left, left);

  puts("Valor 2:");
  printf("  binario: ");
  print_bits(right);
  printf("\n  decimal: %hu\n  octal: %ho\n  hexadecimal: 0x%04hx\n",
         right, right, right);

  puts("Operacoes:");
  printf("  %hu & %hu = %hu\n", left, right, and_result);
  printf("  %hu | %hu = %hu\n", left, right, or_result);
  printf("  %hu ^ %hu = %hu\n", left, right, xor_result);
  printf("  %hu & ~%hu = %hu\n", left, right, clear_result);
  return 0;
}