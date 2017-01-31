#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>
#include "crazy_sqlite_writer.h"

static const char usage_string[] =
    "crazy_sqlite_writer [-h|--help] [--max-size <size>] <work dir>";

void usage(FILE *f) {
  fprintf(f, "usage: %s\n", usage_string);
}

void error(const char *err, ...) {
  fputs(PROG_NAME " error: ", stderr);
  va_list params;
  va_start(params, err);
  vfprintf(stderr, err, params);
  va_end(params);
  fputc('\n', stderr);
}

noreturn void fatal(int status, const char *err, ...) {
  fputs(PROG_NAME " fatal: ", stderr);
  va_list params;
  va_start(params, err);
  vfprintf(stderr, err, params);
  va_end(params);
  fputc('\n', stderr);
  exit(status);
}

void fatal_no_exit(const char *err, ...) {
  fputs(PROG_NAME " fatal: ", stderr);
  va_list params;
  va_start(params, err);
  vfprintf(stderr, err, params);
  va_end(params);
  fputc('\n', stderr);
}
