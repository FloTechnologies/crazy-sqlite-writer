#ifndef CRAZY_SQLITE_WRITER_CRAZY_SQLITE_WRITER_H
#define CRAZY_SQLITE_WRITER_CRAZY_SQLITE_WRITER_H

#include <stdbool.h>

struct opt {
  const char prog_name[256];
  const char sqlite_filename[256];
  unsigned long long max_size;
  const char *work_dir;
  bool usage;
} opt = {
    .prog_name = "crazy-sqlite-writer",
    .sqlite_filename = "crazy-sqlite-writer.db",
    .max_size = 1 << 30 /* 1 GB */,
    .usage = false
};

#endif //CRAZY_SQLITE_WRITER_CRAZY_SQLITE_WRITER_H
