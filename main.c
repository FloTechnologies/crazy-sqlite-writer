#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <path-join/path-join.h>
#include "crazy_sqlite_writer.h"
#include "usage.h"

unsigned long long parse_human_readable_size(const char *sz) {
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

bool parse_args(const int argc, const char *argv[]) {
  static struct option long_options[] = {
      { "help",     no_argument,       0, 'h' },
      { "max-size", required_argument, 0, 0   },
      { 0,          0,                 0, 0   }
  };
  int c;

  for (;;) {
    int option_index = 0;

    c = getopt_long(argc, (char *const *) argv, "h",
        long_options, &option_index);

    if (c == -1)
      break;

    if (c == '?' || c == ':')
      return false;

    if (c == 'h') {
      opt.usage = true;
      return true;
    }

    switch (option_index) {
      case 1:
        opt.max_size = parse_human_readable_size(optarg);
        break;
      default:
        fatal(EINVAL, "failed to parse the arguments");
    }
  }

  if (opt.usage)
    return true;

  if (optind == argc || optind + 1 < argc)
    return EINVAL;

  opt.work_dir = argv[optind];

  return true;
}

bool validate_dir(const char *dir) {
  bool ret = false;
  struct stat sb;

  if (stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode))
    ret = true;

  return ret;
}

int do_job() {
  int ret = 0;
  struct sqlite3 *db;
  char *errmsg = 0;
  int rc;

  rc = sqlite3_open(path_join(opt.work_dir, opt.sqlite_filename), &db);

  for (;;) {

    for (;;) {
      break;
    }
    break;
  }

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

  if (!validate_dir(opt.work_dir)) {
    fatal_no_exit("invalid work dir");
    return EINVAL;
  }

  return do_job();
}