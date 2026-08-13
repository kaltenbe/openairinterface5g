% Load GNB_PHY_UL_FREQ_CHANNEL_ESTIMATE records from an HDF5 extraction.
%
% Set these variables before running the script, or edit the defaults below:
%   h5_filename  - file produced by extract -format hdf5
%   target_rnti  - RNTI to load. Required when the file contains several.
%   expected_Nfft - optional expected number of complex samples per record
%   Nmeasurements - optional maximum number of measurements to load
%
% Outputs:
%   channel_estimates - complex double [Nfft,Nport,Nantenna,Nmeasurements]
%   measurement_present - logical [Nport,Nantenna,Nmeasurements], indicating
%                         which entries were present rather than zero-filled
%   selected_rnti     - the single RNTI included in channel_estimates
%   measurement_frame, measurement_subframe, measurement_timestamp
%
% Antenna and port values in the trace are zero-based. They are stored at
% antenna+1 and port+1 in the MATLAB array. Missing antenna/port records,
% including incomplete measurements at the start of a capture, remain zero.

if ~exist("h5_filename", "var")
    h5_filename = "channel_estimates.h5";
end
if ~exist("target_rnti", "var")
    target_rnti = [];
end
if ~exist("expected_Nfft", "var")
    expected_Nfft = [];
end
if ~exist("Nmeasurements", "var")
    Nmeasurements = [];
end

event_group = "/events/GNB_PHY_UL_FREQ_CHANNEL_ESTIMATE";

rnti_all = h5read(h5_filename, event_group + "/rnti");
frame_all = h5read(h5_filename, event_group + "/frame");
subframe_all = h5read(h5_filename, event_group + "/subframe");
antenna_all = h5read(h5_filename, event_group + "/antenna");
port_all = h5read(h5_filename, event_group + "/port");
timestamp_sec_all = h5read(h5_filename, event_group + "/timestamp_sec");
timestamp_nsec_all = h5read(h5_filename, event_group + "/timestamp_nsec");
offsets = h5read(h5_filename, event_group + "/chest_f_offsets");

% Work with column vectors regardless of how a particular MATLAB release
% presents one-dimensional HDF5 datasets.
rnti_all = rnti_all(:);
frame_all = frame_all(:);
subframe_all = subframe_all(:);
antenna_all = antenna_all(:);
port_all = port_all(:);
timestamp_sec_all = timestamp_sec_all(:);
timestamp_nsec_all = timestamp_nsec_all(:);
offsets = offsets(:);

record_count = numel(rnti_all);
if record_count == 0
    error("The HDF5 file contains no channel-estimate records.");
end
if numel(offsets) ~= record_count + 1
    error("Invalid HDF5 file: expected one more buffer offset than records.");
end

available_rntis = unique(rnti_all);
if isempty(target_rnti)
    if numel(available_rntis) ~= 1
        error("The file contains multiple RNTIs (%s). Set target_rnti explicitly.", ...
              strtrim(sprintf("%d ", available_rntis)));
    end
    selected_rnti = available_rntis(1);
else
    if ~isscalar(target_rnti)
        error("target_rnti must be a scalar.");
    end
    if ~any(available_rntis == target_rnti)
        error("RNTI %d is not present. Available RNTIs: %s", ...
              target_rnti, strtrim(sprintf("%d ", available_rntis)));
    end
    selected_rnti = cast(target_rnti, "like", rnti_all);
end

record_indices = find(rnti_all == selected_rnti);
frames = frame_all(record_indices);
subframes = subframe_all(record_indices);
antennas = double(antenna_all(record_indices));
ports = double(port_all(record_indices));

if any(antennas < 0) || any(antennas ~= fix(antennas)) || ...
   any(ports < 0) || any(ports ~= fix(ports))
    error("Antenna and port identifiers must be nonnegative integers.");
end

Nantenna = max(antennas) + 1;
Nport = max(ports) + 1;

% Records for one measurement are emitted consecutively. A new measurement
% begins when frame/subframe changes or the producer's antenna/port ordering
% resets. The ordering-reset rule also separates consecutive measurements with
% identical frame metadata and handles an incomplete first measurement.
measurement_index = zeros(numel(record_indices), 1);
measurement_count = 0;
previous_frame = [];
previous_subframe = [];
previous_pair_order = [];

for k = 1:numel(record_indices)
    antenna_index = antennas(k) + 1;
    port_index = ports(k) + 1;
    pair_order = antennas(k) * Nport + ports(k);
    starts_new_measurement = measurement_count == 0 || ...
        frames(k) ~= previous_frame || ...
        subframes(k) ~= previous_subframe || ...
        pair_order <= previous_pair_order;

    if starts_new_measurement
        measurement_count = measurement_count + 1;
        previous_frame = frames(k);
        previous_subframe = subframes(k);
    end
    measurement_index(k) = measurement_count;
    previous_pair_order = pair_order;
end

available_measurements = measurement_count;
if isempty(Nmeasurements)
    Nmeasurements = available_measurements;
else
    if ~isscalar(Nmeasurements) || Nmeasurements <= 0 || ...
       Nmeasurements ~= fix(Nmeasurements)
        error("Nmeasurements must be a positive integer or empty.");
    end
    Nmeasurements = min(double(Nmeasurements), available_measurements);
end

% Discard records beyond the requested limit before reading chest_f_data.
keep = measurement_index <= Nmeasurements;
record_indices = record_indices(keep);
frames = frames(keep);
subframes = subframes(keep);
antennas = antennas(keep);
ports = ports(keep);
measurement_index = measurement_index(keep);
measurement_count = Nmeasurements;

% Each c16_t sample contains int16 real and imaginary parts (four bytes).
% Validate only the records that will actually be loaded.
byte_counts = double(offsets(record_indices + 1) - offsets(record_indices));
if any(mod(byte_counts, 4) ~= 0)
    error("A chest_f record has a size that is not a multiple of sizeof(c16_t).");
end
sample_counts = byte_counts / 4;

if isempty(expected_Nfft)
    distinct_sample_counts = unique(sample_counts);
    if numel(distinct_sample_counts) ~= 1
        error(["Records have different sample counts (%s). Set expected_Nfft " + ...
               "or extract a homogeneous capture."], ...
              strtrim(sprintf("%d ", distinct_sample_counts)));
    end
    Nfft = distinct_sample_counts(1);
else
    if ~isscalar(expected_Nfft) || expected_Nfft <= 0 || expected_Nfft ~= fix(expected_Nfft)
        error("expected_Nfft must be a positive integer.");
    end
    Nfft = double(expected_Nfft);
    if any(sample_counts ~= Nfft)
        error("At least one loaded record does not contain expected_Nfft=%d samples.", Nfft);
    end
end

channel_estimates = complex(zeros(Nfft, Nport, Nantenna, measurement_count));
measurement_present = false(Nport, Nantenna, measurement_count);
measurement_frame = zeros(measurement_count, 1, "like", frame_all);
measurement_subframe = zeros(measurement_count, 1, "like", subframe_all);
measurement_timestamp = NaT(measurement_count, 1, "TimeZone", "UTC");

[~, ~, machine_endian] = computer;
for k = 1:numel(record_indices)
    record = record_indices(k);
    byte_offset = double(offsets(record));
    byte_count = double(offsets(record + 1) - offsets(record));

    bytes = h5read(h5_filename, event_group + "/chest_f_data", ...
                   byte_offset + 1, byte_count);
    iq = typecast(uint8(bytes(:)), "int16");
    if machine_endian == 'B'
        iq = swapbytes(iq);
    end
    iq = iq(:);
    channel = complex(double(iq(1:2:end)), double(iq(2:2:end)));

    antenna_index = antennas(k) + 1;
    port_index = ports(k) + 1;
    measurement = measurement_index(k);
    channel_estimates(:, port_index, antenna_index, measurement) = channel;
    measurement_present(port_index, antenna_index, measurement) = true;

    % Set measurement-level metadata from its first record.
    if isnat(measurement_timestamp(measurement))
        measurement_frame(measurement) = frame_all(record);
        measurement_subframe(measurement) = subframe_all(record);
        measurement_timestamp(measurement) = ...
            datetime(double(timestamp_sec_all(record)), ...
                     "ConvertFrom", "posixtime", "TimeZone", "UTC") + ...
            seconds(double(timestamp_nsec_all(record)) * 1e-9);
    end
end

fprintf("Loaded RNTI %d: [%d x %d x %d x %d]\n", ...
        selected_rnti, Nfft, Nport, Nantenna, measurement_count);
