#ifndef CRAZY_SQLITE_WRITER_USAGE_H
#define CRAZY_SQLITE_WRITER_USAGE_H

void usage(FILE *f);
void error(const char *err, ...);
void fatal(int status, const char *err, ...);
void fatal_no_exit(const char *err, ...);

#endif //CRAZY_SQLITE_WRITER_USAGE_H
