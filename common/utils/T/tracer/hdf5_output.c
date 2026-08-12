/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "hdf5_output.h"
#include <hdf5.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HDF5_FORMAT_VERSION 1
#define ROW_CHUNK_SIZE 1024
#define DATA_CHUNK_SIZE (1024 * 1024)

typedef struct {
  hid_t dataset;
  hid_t memory_type;
  enum event_arg_type type;
  int event_arg;
} field_dataset;

struct hdf5_output {
  hid_t file;
  hid_t group;
  hid_t timestamp_sec;
  hid_t timestamp_nsec;
  hid_t buffer_data;
  hid_t buffer_offsets;
  field_dataset *fields;
  int field_count;
  int buffer_arg;
  uint64_t records;
  uint64_t data_size;
};

static int write_string_attribute(hid_t object, const char *name, const char *value)
{
  hid_t space = -1;
  hid_t type = -1;
  hid_t attribute = -1;
  int ret = -1;

  if (value == NULL)
    value = "";
  space = H5Screate(H5S_SCALAR);
  type = H5Tcopy(H5T_C_S1);
  if (space < 0 || type < 0 || H5Tset_size(type, strlen(value) + 1) < 0 || H5Tset_strpad(type, H5T_STR_NULLTERM) < 0)
    goto out;
  attribute = H5Acreate2(object, name, type, space, H5P_DEFAULT, H5P_DEFAULT);
  if (attribute < 0 || H5Awrite(attribute, type, value) < 0)
    goto out;
  ret = 0;
out:
  if (attribute >= 0) H5Aclose(attribute);
  if (type >= 0) H5Tclose(type);
  if (space >= 0) H5Sclose(space);
  return ret;
}

static int write_scalar_attribute(hid_t object, const char *name, hid_t type, const void *value)
{
  hid_t space = H5Screate(H5S_SCALAR);
  if (space < 0)
    return -1;
  hid_t attribute = H5Acreate2(object, name, type, space, H5P_DEFAULT, H5P_DEFAULT);
  int ret = attribute >= 0 && H5Awrite(attribute, type, value) >= 0 ? 0 : -1;
  if (attribute >= 0) H5Aclose(attribute);
  H5Sclose(space);
  return ret;
}

static hid_t create_extendible_dataset(hid_t parent, const char *name, hid_t type, hsize_t chunk_size)
{
  hsize_t dims[] = {0};
  hsize_t maxdims[] = {H5S_UNLIMITED};
  hsize_t chunks[] = {chunk_size};
  hid_t space = H5Screate_simple(1, dims, maxdims);
  hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
  hid_t dataset = -1;
  if (space >= 0 && dcpl >= 0 && H5Pset_chunk(dcpl, 1, chunks) >= 0)
    dataset = H5Dcreate2(parent, name, type, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);
  if (dcpl >= 0) H5Pclose(dcpl);
  if (space >= 0) H5Sclose(space);
  return dataset;
}

static int append_values(hid_t dataset, hid_t memory_type, uint64_t old_size, hsize_t count, const void *values)
{
  hsize_t new_size[] = {old_size + count};
  hsize_t start[] = {old_size};
  hsize_t slab[] = {count};
  if (H5Dset_extent(dataset, new_size) < 0)
    return -1;
  hid_t file_space = H5Dget_space(dataset);
  hid_t memory_space = H5Screate_simple(1, slab, NULL);
  int ret = -1;
  if (file_space >= 0 && memory_space >= 0 &&
      H5Sselect_hyperslab(file_space, H5S_SELECT_SET, start, NULL, slab, NULL) >= 0 &&
      H5Dwrite(dataset, memory_type, memory_space, file_space, H5P_DEFAULT, values) >= 0)
    ret = 0;
  if (memory_space >= 0) H5Sclose(memory_space);
  if (file_space >= 0) H5Sclose(file_space);
  return ret;
}

static enum event_arg_type event_type_from_format(const char *type)
{
  if (!strcmp(type, "int")) return EVENT_INT;
  if (!strcmp(type, "ulong")) return EVENT_ULONG;
  if (!strcmp(type, "float")) return EVENT_FLOAT;
  if (!strcmp(type, "string")) return EVENT_STRING;
  if (!strcmp(type, "buffer")) return EVENT_BUFFER;
  fprintf(stderr, "HDF5 output: unsupported event field type '%s'\n", type);
  return -1;
}

static hid_t file_type_for_event_type(enum event_arg_type type)
{
  switch (type) {
    case EVENT_INT: return H5T_STD_I32LE;
    case EVENT_ULONG: return H5T_STD_U64LE;
    case EVENT_FLOAT: return H5T_IEEE_F32LE;
    default: return -1;
  }
}

static hid_t memory_type_for_event_type(enum event_arg_type type)
{
  switch (type) {
    case EVENT_INT: return H5T_NATIVE_INT32;
    case EVENT_ULONG: return H5T_NATIVE_UINT64;
    case EVENT_FLOAT: return H5T_NATIVE_FLOAT;
    default: return -1;
  }
}

static char *make_format_description(database_event_format format)
{
  size_t size = 1;
  for (int i = 0; i < format.count; ++i)
    size += strlen(format.type[i]) + strlen(format.name[i]) + 4;
  char *description = malloc(size);
  if (description == NULL)
    return NULL;
  description[0] = 0;
  for (int i = 0; i < format.count; ++i) {
    if (i != 0) strcat(description, " : ");
    strcat(description, format.type[i]);
    strcat(description, ",");
    strcat(description, format.name[i]);
  }
  return description;
}

hdf5_output *hdf5_output_open(const char *filename,
                              const hdf5_output_config *config,
                              database_event_format format,
                              int buffer_arg)
{
  hdf5_output *output = calloc(1, sizeof(*output));
  char group_name[1024];
  char *format_description = NULL;
  uint64_t initial_offset = 0;
  int version = HDF5_FORMAT_VERSION;
  int ulong_size = sizeof(unsigned long);

  if (output == NULL)
    return NULL;
  output->file = output->group = output->timestamp_sec = output->timestamp_nsec = -1;
  output->buffer_data = output->buffer_offsets = -1;

  output->file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if (output->file < 0)
    goto error;
  hid_t events_group = H5Gcreate2(output->file, "/events", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  if (events_group < 0)
    goto error;
  H5Gclose(events_group);
  snprintf(group_name, sizeof(group_name), "/events/%s", config->event_name);
  output->group = H5Gcreate2(output->file, group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  if (output->group < 0)
    goto error;

  format_description = make_format_description(format);
  int64_t after_sec = config->after_sec;
  int64_t after_nsec = config->after_nsec;
  if (format_description == NULL ||
      write_scalar_attribute(output->file, "format_version", H5T_NATIVE_INT, &version) < 0 ||
      write_string_attribute(output->file, "source_file", config->source_file) < 0 ||
      write_string_attribute(output->file, "database_file", config->database_file) < 0 ||
      write_string_attribute(output->group, "event_name", config->event_name) < 0 ||
      write_scalar_attribute(output->group, "event_id", H5T_NATIVE_INT, &config->event_id) < 0 ||
      write_string_attribute(output->group, "selected_buffer", config->buffer_name) < 0 ||
      write_string_attribute(output->group, "event_format", format_description) < 0 ||
      write_string_attribute(output->group, "filters", config->filters) < 0 ||
      write_scalar_attribute(output->group, "sizeof_unsigned_long", H5T_NATIVE_INT, &ulong_size) < 0 ||
      write_scalar_attribute(output->group, "after_filter_enabled", H5T_NATIVE_INT, &config->check_time) < 0 ||
      write_scalar_attribute(output->group, "after_sec", H5T_NATIVE_INT64, &after_sec) < 0 ||
      write_scalar_attribute(output->group, "after_nsec", H5T_NATIVE_INT64, &after_nsec) < 0)
    goto error;
  free(format_description);
  format_description = NULL;

  output->timestamp_sec = create_extendible_dataset(output->group, "timestamp_sec", H5T_STD_I64LE, ROW_CHUNK_SIZE);
  output->timestamp_nsec = create_extendible_dataset(output->group, "timestamp_nsec", H5T_STD_I64LE, ROW_CHUNK_SIZE);

  char data_name[512];
  char offsets_name[512];
  snprintf(data_name, sizeof(data_name), "%s_data", config->buffer_name);
  snprintf(offsets_name, sizeof(offsets_name), "%s_offsets", config->buffer_name);
  output->buffer_data = create_extendible_dataset(output->group, data_name, H5T_STD_U8LE, DATA_CHUNK_SIZE);
  output->buffer_offsets = create_extendible_dataset(output->group, offsets_name, H5T_STD_U64LE, ROW_CHUNK_SIZE);
  if (output->timestamp_sec < 0 || output->timestamp_nsec < 0 || output->buffer_data < 0 || output->buffer_offsets < 0 ||
      append_values(output->buffer_offsets, H5T_NATIVE_UINT64, 0, 1, &initial_offset) < 0)
    goto error;

  output->fields = calloc(format.count, sizeof(*output->fields));
  if (output->fields == NULL)
    goto error;
  output->buffer_arg = buffer_arg;
  for (int i = 0; i < format.count; ++i) {
    enum event_arg_type type = event_type_from_format(format.type[i]);
    if (type == EVENT_BUFFER) {
      if (i != buffer_arg)
        fprintf(stderr, "HDF5 output: skipping additional buffer field '%s'\n", format.name[i]);
      continue;
    }
    field_dataset *field = &output->fields[output->field_count];
    hid_t file_type;
    if (type == EVENT_STRING) {
      file_type = H5Tcopy(H5T_C_S1);
      if (file_type < 0 || H5Tset_size(file_type, H5T_VARIABLE) < 0 || H5Tset_cset(file_type, H5T_CSET_UTF8) < 0)
        goto error;
      field->memory_type = file_type;
    } else {
      file_type = file_type_for_event_type(type);
      field->memory_type = memory_type_for_event_type(type);
    }
    field->dataset = create_extendible_dataset(output->group, format.name[i], file_type, ROW_CHUNK_SIZE);
    if (field->dataset < 0) {
      if (type == EVENT_STRING) H5Tclose(file_type);
      goto error;
    }
    field->type = type;
    field->event_arg = i;
    output->field_count++;
  }
  return output;

error:
  free(format_description);
  hdf5_output_close(output);
  return NULL;
}

int hdf5_output_append(hdf5_output *output, const event *e)
{
  int64_t sec = e->sending_time.tv_sec;
  int64_t nsec = e->sending_time.tv_nsec;
  if (append_values(output->timestamp_sec, H5T_NATIVE_INT64, output->records, 1, &sec) < 0 ||
      append_values(output->timestamp_nsec, H5T_NATIVE_INT64, output->records, 1, &nsec) < 0)
    return -1;

  for (int i = 0; i < output->field_count; ++i) {
    field_dataset *field = &output->fields[i];
    const event_arg *arg = &e->e[field->event_arg];
    const void *value;
    int32_t int_value;
    uint64_t ulong_value;
    switch (field->type) {
      case EVENT_INT:
        int_value = arg->i;
        value = &int_value;
        break;
      case EVENT_ULONG:
        ulong_value = arg->ul;
        value = &ulong_value;
        break;
      case EVENT_FLOAT:
        value = &arg->f;
        break;
      case EVENT_STRING:
        value = &arg->s;
        break;
      default:
        return -1;
    }
    if (append_values(field->dataset, field->memory_type, output->records, 1, value) < 0)
      return -1;
  }

  if (output->buffer_arg < 0 || output->buffer_arg >= e->ecount || e->e[output->buffer_arg].type != EVENT_BUFFER)
    return -1;
  const event_arg *buffer = &e->e[output->buffer_arg];
  if (buffer->bsize > 0 && append_values(output->buffer_data, H5T_NATIVE_UCHAR, output->data_size, buffer->bsize, buffer->b) < 0)
    return -1;
  output->data_size += buffer->bsize;
  if (append_values(output->buffer_offsets, H5T_NATIVE_UINT64, output->records + 1, 1, &output->data_size) < 0)
    return -1;
  output->records++;
  return 0;
}

int hdf5_output_close(hdf5_output *output)
{
  int ret = 0;
  if (output == NULL)
    return 0;
  if (output->group >= 0 && write_scalar_attribute(output->group, "record_count", H5T_NATIVE_UINT64, &output->records) < 0)
    ret = -1;
  for (int i = 0; i < output->field_count; ++i) {
    if (output->fields[i].dataset >= 0 && H5Dclose(output->fields[i].dataset) < 0) ret = -1;
    if (output->fields[i].type == EVENT_STRING && output->fields[i].memory_type >= 0 &&
        H5Tclose(output->fields[i].memory_type) < 0) ret = -1;
  }
  free(output->fields);
  if (output->buffer_offsets >= 0 && H5Dclose(output->buffer_offsets) < 0) ret = -1;
  if (output->buffer_data >= 0 && H5Dclose(output->buffer_data) < 0) ret = -1;
  if (output->timestamp_nsec >= 0 && H5Dclose(output->timestamp_nsec) < 0) ret = -1;
  if (output->timestamp_sec >= 0 && H5Dclose(output->timestamp_sec) < 0) ret = -1;
  if (output->group >= 0 && H5Gclose(output->group) < 0) ret = -1;
  if (output->file >= 0 && H5Fclose(output->file) < 0) ret = -1;
  free(output);
  return ret;
}
