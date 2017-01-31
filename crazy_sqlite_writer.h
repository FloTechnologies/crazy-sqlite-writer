#ifndef CRAZY_SQLITE_WRITER_CRAZY_SQLITE_WRITER_H
#define CRAZY_SQLITE_WRITER_CRAZY_SQLITE_WRITER_H

#include <stdbool.h>
#include "version.h"

#define PROG_NAME "crazy-sqlite-writer"
#define SQLITE_FILENAME "crazy-sqlite-writer.db"

struct opt {
  const char prog_name[sizeof PROG_NAME];
  const char sqlite_filename[sizeof SQLITE_FILENAME];
  unsigned long long max_size;
  const char *work_dir;
  bool usage;
  bool version;
};

extern struct opt opt;

#endif //CRAZY_SQLITE_WRITER_CRAZY_SQLITE_WRITER_H
