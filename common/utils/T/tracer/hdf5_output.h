/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef T_TRACER_HDF5_OUTPUT_H
#define T_TRACER_HDF5_OUTPUT_H

#include "database.h"
#include "event.h"

typedef struct hdf5_output hdf5_output;

typedef struct {
  const char *source_file;
  const char *database_file;
  const char *event_name;
  int event_id;
  const char *buffer_name;
  const char *filters;
  int check_time;
  time_t after_sec;
  long after_nsec;
} hdf5_output_config;

hdf5_output *hdf5_output_open(const char *filename,
                              const hdf5_output_config *config,
                              database_event_format format,
                              int buffer_arg);
int hdf5_output_append(hdf5_output *output, const event *e);
int hdf5_output_close(hdf5_output *output);

#endif /* T_TRACER_HDF5_OUTPUT_H */
