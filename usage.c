#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>

const char usage_string[] =
    "crazy_sqlite_writer [-h|--help] [--max-size <size>] <work dir>";

void usage(FILE *f) {
  fprintf(f, "usage: %s\n", usage_string);
}

noreturn void fatal(int status, const char *err, ...) {
  fputs("fatal: ", stderr);
  va_list params;
  va_start(params, err);
  vfprintf(stderr, err, params);
  va_end(params);
  fputc('\n', stderr);
  exit(status);
}

void fatal_no_exit(const char *err, ...) {
  fputs("fatal: ", stderr);
  va_list params;
  va_start(params, err);
  vfprintf(stderr, err, params);
  va_end(params);
  fputc('\n', stderr);
}
