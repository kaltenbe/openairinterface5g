<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# Extract buffers to HDF5

The `extract` tracer tool can optionally write matching events to an HDF5 file.
Unlike its raw output, HDF5 output preserves each buffer boundary, the event
timestamp, and all non-buffer fields declared by the event's `FORMAT` in
`T_messages.txt`.

## Build

Enable HDF5 support in a CMake build with:

```shell
cmake -DT_EXTRACT_HDF5=ON .
make extract
```

The HDF5 C development library must be installed. For a legacy Makefile build,
use `make HDF5=1 extract`; this obtains its flags from `pkg-config hdf5`.

## Usage

The event selection and filtering options are the same as for raw extraction.
Select HDF5 output with `-format hdf5`:

```shell
./extract -d ../T_messages.txt \
  recording.raw GNB_PHY_UL_FREQ_CHANNEL_ESTIMATE chest_f \
  -o channel_estimates.h5 -format hdf5
```

For this event, the file contains:

```text
/events/GNB_PHY_UL_FREQ_CHANNEL_ESTIMATE/
  timestamp_sec
  timestamp_nsec
  gNB_ID
  rnti
  frame
  subframe
  antenna
  port
  chest_f_data
  chest_f_offsets
```

The schema is generated from `T_messages.txt`, so other events get datasets for
their own fields. Scalar datasets have one value per matched event. Strings are
stored as variable-length UTF-8 strings. The selected buffer is stored as one
flat byte dataset plus offsets; buffer `i` occupies
`data[offsets[i]:offsets[i + 1]]`.

The event group also records the event ID, selected buffer, complete event
format, applied filters, source paths, and final record count as attributes.
Raw output remains the default when `-format` is omitted.

For `GNB_PHY_UL_FREQ_CHANNEL_ESTIMATE`, the MATLAB script
`common/utils/T/tracer/load_channel_estimates_hdf5.m` loads one selected RNTI
into an `[Nfft,Nport,Nantenna,Nmeasurements]` complex array. Missing
antenna/port combinations are zero-filled and reported separately in its
`measurement_present` output. Set the optional `Nmeasurements` input variable
before running the script to limit how many measurements and channel buffers
are read.
