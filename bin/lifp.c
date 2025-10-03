#include "../lib/alloc.h"
#include "../vendor/args/src/args.h"

// NOLINTBEGIN - intentionally including .c files
#include "../cmd/repl.c"
#include "../cmd/run.c"
// NOLINTEND

#include <stddef.h>
#include <stdio.h>
#include <string.h>

constexpr int KILOBYTE = 1024;

allocMetricsInit();

void formatRootUsage(size_t size, char buffer[static size]) {
  snprintf(buffer, size,
           "Usage:\n"
           "  %s <command> [flags] [args]\n"
           "\n"
           "Commands:\n"
           "  run     file.lifp            run a file with lifp\n"
           "  repl                         start a REPL session\n"
           "  help    command              show help for command\n"
           "\n"
           "Flags:\n"
           "  -h, --help                   print this help and exit\n"
           "  -v, --version                print lifp version and exit\n"
           "\n"
           "Learn more about lifp at https://github.com/shikaan/lifp",
           NAME);
}

void formatRunUsage(size_t size, char buffer[static size]) {
  snprintf(buffer, size,
           "Usage:\n"
           "  %s %s [flags] file.lifp\n"
           "\n"
           "Flags:\n"
           "  -f, --file-size         int    max size of source file (in KB)\n"
           "  -a, --ast-memory        int    max parsing memory size (in KB)\n"
           "  -h, --help                     print this help and exit\n"
           "  -v, --version                  print version and exit\n"
           "\n"
           "Learn more about lifp at https://github.com/shikaan/lifp",
           NAME, RUN);
}

void formatReplUsage(size_t size, char buffer[static size]) {
  snprintf(buffer, size,
           "Usage:\n"
           "  %s %s [flags]\n"
           "\n"
           "Flags:\n"
           "  -o, --output-size       int    max length of the output buffer\n"
           "  -a, --ast-memory        int    max parsing memory size (in KB)\n"
           "  -h, --help                     print this help and exit\n"
           "  -v, --version                  print version and exit\n"
           "\n"
           "Learn more about lifp at https://github.com/shikaan/lifp",
           NAME, REPL);
}

int runCallback(char *name, ArgParser *parser) {
  int count = ap_count_args(parser);
  if (count != 1) {
    error("%s requires 1 argument", name);
    return 1;
  }

  run_opts_t opts;
  opts.ast_memory = (size_t)ap_get_int_value(parser, "ast-memory") * KILOBYTE;
  opts.file_size = (size_t)ap_get_int_value(parser, "file-size") * KILOBYTE;
  opts.filename = ap_get_arg_at_index(parser, 0);

  return run(opts);
}

int replCallback(char *name, ArgParser *parser) {
  (void)name;
  repl_opts_t opts;
  opts.ast_memory = (size_t)ap_get_int_value(parser, "ast-memory") * KILOBYTE;
  opts.output_size = (size_t)ap_get_int_value(parser, "output-size");

  return repl(opts);
}

int main(int argc, char **argv) {
  ArgParser *root_parser = nullptr;
  ArgParser *run_parser = nullptr;
  ArgParser *repl_parser = nullptr;

  root_parser = ap_new_parser();
  if (!root_parser) {
    error("unable to allocate memory");
    goto error;
  }

  char usage_help[512];
  formatRootUsage(512, usage_help);
  char version[512];
  formatVersion(512, version);
  char run_help[512];
  formatRunUsage(512, run_help);
  char repl_help[512];
  formatReplUsage(512, repl_help);

  ap_set_helptext(root_parser, usage_help);
  ap_set_version(root_parser, version);

  // RUN
  run_parser = ap_new_cmd(root_parser, RUN);
  if (!run_parser) {
    error("unable to allocate memory");
    goto error;
  }

  ap_set_helptext(run_parser, run_help);
  ap_set_version(run_parser, version);
  ap_add_int_opt(run_parser, "file-size f", 1024);
  ap_add_int_opt(run_parser, "ast-memory a", 128);

  ap_set_cmd_callback(run_parser, runCallback);

  // REPL
  repl_parser = ap_new_cmd(root_parser, REPL);
  if (!repl_parser) {
    error("unable to allocate memory");
    goto error;
  }

  ap_set_helptext(repl_parser, repl_help);
  ap_set_version(repl_parser, version);
  ap_add_int_opt(repl_parser, "output-size o", 4096);
  ap_add_int_opt(repl_parser, "ast-memory a", 128);

  ap_set_cmd_callback(repl_parser, replCallback);

  ap_enable_help_command(root_parser, true);
  if (!ap_parse(root_parser, argc, argv)) {
    error("unable to parse command line arguments");
    goto error;
  }

  if (!ap_found_cmd(root_parser)) {
    printf("%s", ap_get_helptext(root_parser));
    return 1;
  }

  ap_free(repl_parser);
  ap_free(run_parser);
  return ap_get_cmd_exit_code(root_parser);

error:
  ap_free(repl_parser);
  ap_free(run_parser);
  ap_free(root_parser);
  return 1;
}
