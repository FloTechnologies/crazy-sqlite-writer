#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sqlite3.h>
#include <path-join/path-join.h>
#include <stdlib.h>
#include <time.h>
#include "crazy_sqlite_writer.h"
#include "usage.h"
#include "version.h"

struct opt opt = {
    .prog_name = PROG_NAME,
    .sqlite_filename = SQLITE_FILENAME,
    .max_size = 1 << 30,  /* 1 GB */
    .interval = 500,
    .usage = false,
    .version = false,
};

static unsigned long long parse_human_readable_size(const char *sz) {
  unsigned long long ret = 0;
  int n = 0;

  n = sscanf(sz, "%llu", &ret);

  if (n == 0 || errno) {
    fatal(errno ? errno : EINVAL, "failed to parse size");
  }

  if (strchr(sz, 'k') || strchr(sz, 'K')) {
    ret *= 1 << 10;
  } else if (strchr(sz, 'm') || strchr(sz, 'M')) {
    ret *= 1 << 20;
  } else if (strchr(sz, 'g') || strchr(sz, 'G')) {
    ret *= 1 << 30;
  }

  return ret;
}

static bool parse_args(const int argc, const char *argv[]) {
  static struct option long_options[] = {
      { "help",     no_argument,       0, 'h' },
      { "version",  no_argument,       0, 'v' },
      { "max-size", required_argument, 0, 0   },
      { "interval", required_argument, 0, 0   },
      { 0,          0,                 0, 0   }
  };
  int c;

  for (;;) {
    int option_index = 0;

    c = getopt_long(argc, (char *const *) argv, "hv",
        long_options, &option_index);

    if (c == -1)
      break;

    if (c == '?' || c == ':')
      return false;

    if (c == 'h') {
      opt.usage = true;
      return true;
    }

    if (c == 'v') {
      opt.version = true;
      return true;
    }

    switch (option_index) {
      case 2:
        opt.max_size = parse_human_readable_size(optarg);
        break;
      case 3:
        sscanf(optarg, "%hu", &opt.interval);
        if (opt.interval <= 0 || opt.interval >= 1000)
          fatal(EINVAL, "interval needs to be in this range:\n"
              "                           0 < interval < 1000");
        break;
      default:
        fatal(EINVAL, "failed to parse the arguments");
    }
  }

  if (opt.usage || opt.version)
    return true;

  if (argc == 1 || optind == argc || optind + 1 < argc)
    return EINVAL;

  opt.work_dir = argv[optind];

  return true;
}

static bool validate_dir(const char *dir) {
  bool ret = false;
  struct stat sb;

  if (stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode))
    ret = true;

  return ret;
}

static void proactive_sqlite3_exec(
    sqlite3 *db,
    const char *sql,
    int (*callback) (void *, int, char **, char **),
    void *callback_arg) {
  char *errmsg;

  sqlite3_exec(db, sql, callback, callback_arg, &errmsg);
  if (errmsg) {
    fatal(SQLITE_ERROR, errmsg);
    sqlite3_free(errmsg);
  }
}

static void create_table(sqlite3 *db) {
  const char *create_table_statment =
      "CREATE TABLE \"telemetry\" (\"id\" INTEGER NOT NULL PRIMARY KEY, "
      "\"created_date\" DATETIME NOT NULL, \"device_id\" TEXT, \"flow\" REAL, "
      "\"flow_rate\" REAL, \"pressure\" REAL, \"temperature\" REAL, "
      "\"valve_state\" INTEGER, \"occupied\" INTEGER, \"moist\" INTEGER, "
      "\"raw_data\" TEXT, \"system_mode\" INTEGER, \"zone_mode\" INTEGER, "
      "\"sent_at\" INTEGER);";

  proactive_sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL);
  proactive_sqlite3_exec(db, "PRAGMA foreign_keys=OFF;", NULL, NULL);
  proactive_sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL);
  proactive_sqlite3_exec(db, create_table_statment, NULL, NULL);
  proactive_sqlite3_exec(db, "COMMIT;", NULL, NULL);
}

static void alloc_db_filenames(
    char **db_path, char **db_shm_path, char **db_wal_path) {
  *db_path = path_join(opt.work_dir, opt.sqlite_filename);
  size_t db_path_len = strlen(*db_path);
  *db_shm_path = malloc(sizeof(char *) * (db_path_len + strlen("-shm") + 1));
  *db_wal_path = malloc(sizeof(char *) * (db_path_len + strlen("-wal") + 1));

  memcpy(*db_shm_path, *db_path, db_path_len);
  strcpy(*db_shm_path + db_path_len, "-shm");
  memcpy(*db_wal_path, *db_path, db_path_len);
  strcpy(*db_wal_path + db_path_len, "-wal");
}

static void free_all(size_t *ptr, ...) {
  va_list ap;
  va_start(ap, ptr);
  free(ptr);
  for (;;) {
    size_t *p = va_arg(ap, size_t *);
    if (!p)
      break;
    free(p);
  }
  va_end(ap);
}

static void unlink_all(char *filename, ...) {
  va_list ap;
  va_start(ap, filename);
  unlink(filename);
  for (;;) {
    char *f = va_arg(ap, char *);
    if (!f)
      break;
    unlink(f);
  }
  va_end(ap);
}

static void insert_dummy_telemetry_data(struct sqlite3 *db) {
  proactive_sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL);
  proactive_sqlite3_exec(
      db,
      "INSERT INTO telemetry(\"created_date\", \"device_id\", \"flow\", "
          "\"flow_rate\", \"pressure\", \"temperature\", \"valve_state\", "
          "\"occupied\", \"moist\", \"raw_data\", \"system_mode\", "
          "\"zone_mode\", \"sent_at\") VALUES('2017-01-29 00:00:00.799094', "
          "'8cc7aa027760', 0.0, 0.0, 60.5, 68.0, 1, 0, 0, "
          "'A7485D020000000001', 5, 1, 1485648000);",
      NULL,
      NULL);
  proactive_sqlite3_exec(db, "COMMIT;", NULL, NULL);
}

static int do_job() {
  int ret = 0;
  struct sqlite3 *db = NULL;
  char *errmsg = NULL;
  int rc;
  char *db_path, *db_shm_path, *db_wal_path;
  alloc_db_filenames(&db_path, &db_shm_path, &db_wal_path);
  struct stat st;
  const struct timespec rqtp = {
    .tv_sec = 0,
    .tv_nsec = opt.interval * 1000000L,
  };

  for (;;) {
    unlink_all(db_path, db_shm_path, db_wal_path, NULL);
    if ((rc = sqlite3_open(db_path, &db)) != SQLITE_OK || db == NULL) {
      fatal(rc, "cannot open SQLite database: %s", db_path);
      break;
    }

    create_table(db);

    for (;;) {
      for (int i = 0; i < 100; i++) {
        insert_dummy_telemetry_data(db);
        nanosleep(&rqtp, NULL);
      }

      proactive_sqlite3_exec(db, "PRAGMA wal_checkpoint;", NULL, NULL);
      stat(db_path, &st);
      if (st.st_size >= opt.max_size)
        break;
    }

    sqlite3_close(db);
    nanosleep(&rqtp, NULL);
  }

  free_all(
      (size_t *) db_path,
      (size_t *) db_shm_path,
      (size_t *) db_wal_path,
      NULL);
  return ret;
}

int main(const int argc, const char *argv[]) {
  if (!parse_args(argc, argv)) {
    fatal_no_exit("invalid argument(s)");
    usage(stderr);
    return EINVAL;
  }

  if (opt.usage) {
    usage(stdout);
    return 0;
  }

  if (opt.version) {
    version();
    return 0;
  }

  if (!validate_dir(opt.work_dir)) {
    fatal_no_exit("invalid work dir");
    return EINVAL;
  }

  return do_job();
}
