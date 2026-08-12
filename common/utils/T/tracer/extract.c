/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "database.h"
#include "event.h"
#include "configuration.h"
#ifdef T_EXTRACT_HDF5
#include "hdf5_output.h"
#endif

enum output_format { OUTPUT_RAW, OUTPUT_HDF5 };

void usage(void)
{
  printf(
"usage: [options] <file> <event> <buffer name>\n"
"options:\n"
"    -d <database file>        this option is mandatory\n"
"    -o <output file>          this option is mandatory\n"
"    -format <raw|hdf5>        output format (default: raw)\n"
"    -f <name> <value>         field 'name' of 'event' has to match 'value'\n"
"                              type of 'name' must be int\n"
"                              (you can use several -f options)\n"
"    -after <raw time> <nsec>  'event' time has to be greater than this\n"
"    -count <n>                dump 'n' matching events (less if EOF reached)\n"
"                              (default: no limit)\n"
  );
  exit(1);
}

int get_filter_arg(database_event_format *f, char *field, char *type)
{
  int i;
  for (i = 0; i < f->count; i++)
    if (!strcmp(f->name[i], field)) {
      if (strcmp(f->type[i], type)) break;
      return i;
    }
  printf("bad field %s, check that it exists and has type '%s'\n",field,type);
  exit(1);
}

int main(int n, char **v)
{
  char *database_filename = NULL;
  void *database;
  int i;
  int input_event_id;
  database_event_format f;
  char *file = NULL;
  char *output_file = NULL;
  FILE *out;
#ifdef T_EXTRACT_HDF5
  hdf5_output *hdf5_out = NULL;
#endif
  enum output_format output_format = OUTPUT_RAW;
  int fd;
  char *event_name = NULL;
  char *buffer_name = NULL;
  char *filter[n];
  int filter_arg[n];
  int filter_value[n];
  int filter_count = 0;
  int buffer_arg;
  int found;
  int count = -1;
  int check_time = 0;
  time_t sec = 0;  /* initialization not necessary but gcc is not happy */
  long nsec = 0;   /* initialization not necessary but gcc is not happy */

  for (i = 1; i < n; i++) {
    if (!strcmp(v[i], "-h") || !strcmp(v[i], "--help")) usage();
    if (!strcmp(v[i], "-d"))
      { if (i > n-2) usage(); database_filename = v[++i]; continue; }
    if (!strcmp(v[i], "-o"))
      { if (i > n-2) usage(); output_file = v[++i]; continue; }
    if (!strcmp(v[i], "-format")) { if (i > n-2) usage();
      char *format = v[++i];
      if (!strcmp(format, "raw")) output_format = OUTPUT_RAW;
      else if (!strcmp(format, "hdf5")) output_format = OUTPUT_HDF5;
      else usage();
      continue;
    }
    if (!strcmp(v[i], "-f")) { if (i>n-3) usage();
      filter[filter_count]         = v[++i];
      filter_value[filter_count++] = atoi(v[++i]);
      continue;
    }
    if (!strcmp(v[i], "-after")) { if (i>n-3) usage();
      check_time = 1;
      sec        = atoll(v[++i]);
      nsec       = atol(v[++i]);
      continue;
    }
    if (!strcmp(v[i], "-count"))
      { if (i > n-2) usage(); count = atoi(v[++i]); continue; }
    if (file == NULL) { file = v[i]; continue; }
    if (event_name == NULL) { event_name = v[i]; continue; }
    if (buffer_name == NULL) { buffer_name = v[i]; continue; }
    usage();
  }
  if (file == NULL || event_name == NULL || buffer_name == NULL) usage();

  if (database_filename == NULL) {
    printf("ERROR: provide a database file (-d)\n");
    exit(1);
  }

  if (output_file == NULL) {
    printf("gimme -o <output file>, thanks\n");
    exit(1);
  }

  database = parse_database(database_filename);

  load_config_file(database_filename);

  input_event_id = event_id_from_name(database, event_name);
  f = get_format(database, input_event_id);

  buffer_arg = get_filter_arg(&f, buffer_name, "buffer");

  for (i = 0; i < filter_count; i++)
    filter_arg[i] = get_filter_arg(&f, filter[i], "int");

  out = NULL;
  if (output_format == OUTPUT_RAW) {
    out = fopen(output_file, "wb");
    if (out == NULL) { perror(output_file); exit(1); }
  } else {
#ifdef T_EXTRACT_HDF5
    size_t filters_size = 1;
    for (i = 0; i < filter_count; ++i)
      filters_size += strlen(filter[i]) + 32;
    char *filters_description = calloc(filters_size, 1);
    if (filters_description == NULL) abort();
    for (i = 0; i < filter_count; ++i) {
      size_t used = strlen(filters_description);
      snprintf(filters_description + used,
               filters_size - used,
               "%s%s=%d",
               i == 0 ? "" : ",",
               filter[i],
               filter_value[i]);
    }
    hdf5_output_config config = {
      .source_file = file,
      .database_file = database_filename,
      .event_name = event_name,
      .event_id = input_event_id,
      .buffer_name = buffer_name,
      .filters = filters_description,
      .check_time = check_time,
      .after_sec = sec,
      .after_nsec = nsec,
    };
    hdf5_out = hdf5_output_open(output_file, &config, f, buffer_arg);
    free(filters_description);
    if (hdf5_out == NULL) {
      fprintf(stderr, "ERROR: cannot create HDF5 output file %s\n", output_file);
      exit(1);
    }
#else
    fprintf(stderr, "ERROR: this extract binary was built without HDF5 support\n");
    exit(1);
#endif
  }

  fd = open(file, O_RDONLY);
  if (fd == -1) { perror(file); exit(1); }

  found = 0;

  OBUF ebuf = {.osize = 0, .omaxsize = 0, .obuf = NULL};

  while (1) {
    event e;
    e = get_event(fd, &ebuf, database);
    if (e.type == -1) break;
    if (e.type != input_event_id) continue;
    for (i = 0; i < filter_count; i++)
      if (filter_value[i] != e.e[filter_arg[i]].i)
        break;
    if (i != filter_count)
      continue;
    if (check_time &&
        !(e.sending_time.tv_sec > sec ||
         (e.sending_time.tv_sec == sec && e.sending_time.tv_nsec >= nsec)))
      continue;
    if (output_format == OUTPUT_RAW) {
      if (fwrite(e.e[buffer_arg].b, e.e[buffer_arg].bsize, 1, out) != 1)
        { perror(output_file); exit(1); }
    } else {
#ifdef T_EXTRACT_HDF5
      if (hdf5_output_append(hdf5_out, &e) < 0) {
        fprintf(stderr, "ERROR: writing event to HDF5 file %s\n", output_file);
        hdf5_output_close(hdf5_out);
        exit(1);
      }
#endif
    }
    found++;
    if (count != -1 && found == count)
      break;
  }

  if (found == 0) printf("ERROR: event not found\n");
  if (count != -1 && found != count)
    printf("WARNING: dumped %d events (wanted %d)\n", found, count);

  if (out != NULL)
    fclose(out);
#ifdef T_EXTRACT_HDF5
  if (hdf5_out != NULL && hdf5_output_close(hdf5_out) < 0) {
    fprintf(stderr, "ERROR: closing HDF5 output file %s\n", output_file);
    return 1;
  }
#endif

  return 0;
}
