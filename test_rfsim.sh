#!/usr/bin/env bash
set -euo pipefail

# NR gNB/UE RFsim integration test for the filtered timing-advance loop.
# The RFsim channel distance is ramped through the gNB telnet server. The test
# passes only if TA commands received by the UE compensate the modeled delay.

OAI_BIN_DIR="${OAI_BIN_DIR:-cmake_targets/ran_build/build}"
GNB_BIN="${OAI_BIN_DIR}/nr-softmodem"
UE_BIN="${OAI_BIN_DIR}/nr-uesoftmodem"
TELNET_LIB="${OAI_BIN_DIR}/libtelnetsrv.so"

GNB_CONFIG="${GNB_CONFIG:-targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.sa.band78.fr1.106PRB.usrpb210.conf}"
UE_CONFIG="${UE_CONFIG:-targets/PROJECTS/GENERIC-NR-5GC/CONF/ue.conf}"
CHANNEL_CONFIG="${CHANNEL_CONFIG:-targets/PROJECTS/GENERIC-NR-5GC/CONF/channelmod_rfsimu.conf}"

LOG_DIR="${LOG_DIR:-./test-logs}"
GNB_LOG="${LOG_DIR}/gnb.log"
UE_LOG="${LOG_DIR}/ue.log"
TELNET_LOG="${LOG_DIR}/telnet.log"
RAMP_LOG="${LOG_DIR}/distance-ramp.log"

GNB_STARTUP_TIMEOUT="${GNB_STARTUP_TIMEOUT:-30}"
UE_ATTACH_TIMEOUT="${UE_ATTACH_TIMEOUT:-30}"
TELNET_STARTUP_TIMEOUT="${TELNET_STARTUP_TIMEOUT:-15}"
TA_CONVERGENCE_TIMEOUT="${TA_CONVERGENCE_TIMEOUT:-30}"
RAMP_STEP_INTERVAL="${RAMP_STEP_INTERVAL:-2}"

TELNET_HOST="${TELNET_HOST:-127.0.0.1}"
TELNET_PORT="${TELNET_PORT:-9090}"
TELNET_COMMAND_TIMEOUT="${TELNET_COMMAND_TIMEOUT:-3}"
CHANNEL_MODEL="${CHANNEL_MODEL:-rfsimu_channel_ue0}"

# At 61.44 Msps, one TA command unit (16 samples) represents about 78 m.
# The 500 m default step is well below the default 16-unit TA outlier gate.
SAMPLE_RATE_HZ="${SAMPLE_RATE_HZ:-61440000}"
RAMP_START_DISTANCE_M="${RAMP_START_DISTANCE_M:-0}"
RAMP_END_DISTANCE_M="${RAMP_END_DISTANCE_M:-2500}"
RAMP_STEP_DISTANCE_M="${RAMP_STEP_DISTANCE_M:-500}"
TA_TOLERANCE_COMMANDS="${TA_TOLERANCE_COMMANDS:-3}"
TA_SAMPLES_PER_COMMAND="${TA_SAMPLES_PER_COMMAND:-16}"

GNB_READY_PATTERN="Command line parameters for OAI UE"
UE_ATTACHED_PATTERN="State = NR_RRC_CONNECTED"

GNB_PID=""
UE_PID=""
TEST_GNB_CONFIG=""
TELNET_HISTORY=""

UE_ARGS=(
  -r 106
  --numerology 1
  --band 78
  -C 3619200000
  --uicc0.imsi 001010000000001
  --rfsim
  -O "${UE_CONFIG}"
)

die() {
  echo "ERROR: $*" >&2
  exit 1
}

check_processes() {
  if [[ -n "${GNB_PID}" ]] && ! kill -0 "${GNB_PID}" 2>/dev/null; then
    tail -50 "${GNB_LOG}" || true
    die "gNB terminated unexpectedly"
  fi
  if [[ -n "${UE_PID}" ]] && ! kill -0 "${UE_PID}" 2>/dev/null; then
    tail -50 "${UE_LOG}" || true
    die "UE terminated unexpectedly"
  fi
}

wait_for_pattern() {
  local file="$1"
  local pattern="$2"
  local timeout="$3"
  local description="$4"

  for ((i = 0; i < timeout * 10; i++)); do
    if grep -Fq "${pattern}" "${file}"; then
      return 0
    fi
    check_processes
    sleep 0.1
  done

  tail -50 "${file}" || true
  die "timed out waiting for ${description}"
}

telnet_command() {
  local command="$1"
  local response

  response=$(printf '%s\n' "${command}" | nc -w "${TELNET_COMMAND_TIMEOUT}" "${TELNET_HOST}" "${TELNET_PORT}") ||
    die "telnet command failed: ${command}"
  printf '%s\n' "${response}" >> "${TELNET_LOG}"
  printf '%s\n' "${response}"
}

set_distance() {
  local requested_distance="$1"
  local response
  local exact_distance

  response=$(telnet_command "rfsimu setdistance ${CHANNEL_MODEL} ${requested_distance}")
  if grep -Eqi "error|unmodified" <<< "${response}"; then
    printf '%s\n' "${response}" >&2
    die "RFsim rejected distance ${requested_distance} m"
  fi

  exact_distance=$(sed -n 's/.*new (exact) distance \([0-9.]*\) m.*/\1/p' <<< "${response}" | tail -1)
  [[ -n "${exact_distance}" ]] || die "could not parse RFsim response for distance ${requested_distance} m"
  printf '%s %s\n' "${requested_distance}" "${exact_distance}" >> "${RAMP_LOG}"
  echo "Distance request ${requested_distance} m -> modeled ${exact_distance} m"
}

# Print "number-of-commands cumulative-correction-in-samples" for UE TA CEs
# received after the supplied starting line.
ue_ta_correction() {
  local start_line="$1"
  tail -n "+${start_line}" "${UE_LOG}" |
    awk -v scale="${TA_SAMPLES_PER_COMMAND}" '
      /Received TA_COMMAND/ {
        for (i = 1; i <= NF; i++) {
          if ($i == "TA_COMMAND") {
            command = $(i + 1) + 0
            count++
            correction += (command - 31) * scale
            break
          }
        }
      }
      END { print count + 0, correction + 0 }
    '
}

cleanup() {
  local rc=$?
  trap - EXIT INT TERM

  echo
  echo "Stopping OAI processes..."
  if [[ -n "${UE_PID}" ]] && kill -0 "${UE_PID}" 2>/dev/null; then
    kill -INT "${UE_PID}" 2>/dev/null || true
  fi
  if [[ -n "${GNB_PID}" ]] && kill -0 "${GNB_PID}" 2>/dev/null; then
    kill -INT "${GNB_PID}" 2>/dev/null || true
  fi

  sleep 2

  if [[ -n "${UE_PID}" ]] && kill -0 "${UE_PID}" 2>/dev/null; then
    kill -TERM "${UE_PID}" 2>/dev/null || true
  fi
  if [[ -n "${GNB_PID}" ]] && kill -0 "${GNB_PID}" 2>/dev/null; then
    kill -TERM "${GNB_PID}" 2>/dev/null || true
  fi

  [[ -z "${UE_PID}" ]] || wait "${UE_PID}" 2>/dev/null || true
  [[ -z "${GNB_PID}" ]] || wait "${GNB_PID}" 2>/dev/null || true
  [[ -z "${TEST_GNB_CONFIG}" ]] || rm -f "${TEST_GNB_CONFIG}"
  [[ -z "${TELNET_HISTORY}" ]] || rm -f "${TELNET_HISTORY}"

  echo "Logs: ${GNB_LOG}, ${UE_LOG}, ${TELNET_LOG}, ${RAMP_LOG}"
  return "${rc}"
}

trap cleanup EXIT INT TERM

for command in awk grep nc sed tail; do
  command -v "${command}" >/dev/null || die "required command not found: ${command}"
done

[[ -x "${GNB_BIN}" ]] || die "gNB executable not found: ${GNB_BIN}"
[[ -x "${UE_BIN}" ]] || die "UE executable not found: ${UE_BIN}"
[[ -f "${TELNET_LIB}" ]] || die "telnet server library not found: ${TELNET_LIB}; build it with ./cmake_targets/build_oai --build-lib telnetsrv"
[[ -f "${GNB_CONFIG}" ]] || die "gNB configuration not found: ${GNB_CONFIG}"
[[ -f "${UE_CONFIG}" ]] || die "UE configuration not found: ${UE_CONFIG}"
[[ -f "${CHANNEL_CONFIG}" ]] || die "channel-model configuration not found: ${CHANNEL_CONFIG}"
((RAMP_STEP_DISTANCE_M > 0)) || die "RAMP_STEP_DISTANCE_M must be positive"
((RAMP_END_DISTANCE_M > RAMP_START_DISTANCE_M)) || die "ramp end distance must exceed start distance"
(((RAMP_END_DISTANCE_M - RAMP_START_DISTANCE_M) % RAMP_STEP_DISTANCE_M == 0)) ||
  die "distance range must be an exact multiple of RAMP_STEP_DISTANCE_M"

mkdir -p "${LOG_DIR}"
: > "${GNB_LOG}"
: > "${UE_LOG}"
: > "${TELNET_LOG}"
: > "${RAMP_LOG}"
TELNET_HISTORY=$(mktemp --suffix=.oai-telnet.history)

# Keep the user's gNB config untouched. The temporary copy activates the AWGN
# UL model that setdistance modifies.
GNB_CONFIG_ABS=$(realpath "${GNB_CONFIG}")
GNB_CONFIG_DIR=$(dirname "${GNB_CONFIG_ABS}")
CHANNEL_CONFIG_REL=$(realpath --relative-to="${GNB_CONFIG_DIR}" "${CHANNEL_CONFIG}")
TEST_GNB_CONFIG=$(mktemp "${GNB_CONFIG_DIR}/.ta-rfsim.XXXXXX.conf")
cp "${GNB_CONFIG_ABS}" "${TEST_GNB_CONFIG}"
printf '\n@include "%s"\n' "${CHANNEL_CONFIG_REL}" >> "${TEST_GNB_CONFIG}"

echo "============================================================"
echo "Starting gNB with RF channel model and telnet control"
echo "============================================================"

stdbuf -oL -eL \
  "${GNB_BIN}" \
  -O "${TEST_GNB_CONFIG}" \
  --gNBs.[0].min_rxtxtime 6 \
  --rfsim \
  --rfsimulator.[0].options chanmod \
  --telnetsrv \
  --telnetsrv.listenaddr "${TELNET_HOST}" \
  --telnetsrv.listenport "${TELNET_PORT}" \
  --telnetsrv.histfile "${TELNET_HISTORY}" \
  > >(sed -u 's/^/[gNB] /' | tee "${GNB_LOG}") 2>&1 &
GNB_PID=$!

wait_for_pattern "${GNB_LOG}" "${GNB_READY_PATTERN}" "${GNB_STARTUP_TIMEOUT}" "gNB readiness"

echo "============================================================"
echo "Starting UE"
echo "============================================================"

stdbuf -oL -eL \
  "${UE_BIN}" \
  "${UE_ARGS[@]}" \
  > >(sed -u 's/^/[UE] /' | tee "${UE_LOG}") 2>&1 &
UE_PID=$!

wait_for_pattern "${UE_LOG}" "${UE_ATTACHED_PATTERN}" "${UE_ATTACH_TIMEOUT}" "UE RRC connection"
wait_for_pattern "${GNB_LOG}" "Random channel ${CHANNEL_MODEL} in rfsimulator activated" "${TELNET_STARTUP_TIMEOUT}" "UL channel model"

for ((i = 0; i < TELNET_STARTUP_TIMEOUT * 10; i++)); do
  if response=$(telnet_command "rfsimu getdistance ${CHANNEL_MODEL}" 2>/dev/null) && grep -Fq "${CHANNEL_MODEL}" <<< "${response}"; then
    break
  fi
  check_processes
  sleep 0.1
done
[[ "${response:-}" == *"${CHANNEL_MODEL}"* ]] || die "RFsim telnet channel ${CHANNEL_MODEL} did not become available"

echo "============================================================"
echo "Applying modeled-distance ramp"
echo "============================================================"

set_distance "${RAMP_START_DISTANCE_M}"
sleep "${RAMP_STEP_INTERVAL}"

# Exclude random-access and baseline TA commands from the ramp correction.
UE_TA_START_LINE=$(( $(wc -l < "${UE_LOG}") + 1 ))

for ((distance = RAMP_START_DISTANCE_M + RAMP_STEP_DISTANCE_M;
      distance <= RAMP_END_DISTANCE_M;
      distance += RAMP_STEP_DISTANCE_M)); do
  check_processes
  set_distance "${distance}"
  sleep "${RAMP_STEP_INTERVAL}"
done

FINAL_EXACT_DISTANCE=$(awk 'END { print $2 }' "${RAMP_LOG}")
BASELINE_EXACT_DISTANCE=$(awk 'NR == 1 { print $2 }' "${RAMP_LOG}")
EXPECTED_SAMPLES=$(awk -v end="${FINAL_EXACT_DISTANCE}" -v start="${BASELINE_EXACT_DISTANCE}" -v rate="${SAMPLE_RATE_HZ}" \
  'BEGIN { printf "%d", ((end - start) * rate / 299792458) + 0.5 }')
TOLERANCE_SAMPLES=$((TA_TOLERANCE_COMMANDS * TA_SAMPLES_PER_COMMAND))

echo "Waiting for TA convergence: expected ${EXPECTED_SAMPLES} samples, tolerance +/-${TOLERANCE_SAMPLES}"
converged=0
for ((i = 0; i < TA_CONVERGENCE_TIMEOUT * 2; i++)); do
  check_processes
  read -r TA_COMMAND_COUNT APPLIED_SAMPLES < <(ue_ta_correction "${UE_TA_START_LINE}")
  ERROR_SAMPLES=$((APPLIED_SAMPLES - EXPECTED_SAMPLES))
  ((ERROR_SAMPLES < 0)) && ABS_ERROR_SAMPLES=$((-ERROR_SAMPLES)) || ABS_ERROR_SAMPLES=${ERROR_SAMPLES}
  printf 'TA commands %d, cumulative correction %d samples, error %d samples\n' \
    "${TA_COMMAND_COUNT}" "${APPLIED_SAMPLES}" "${ERROR_SAMPLES}"
  if ((TA_COMMAND_COUNT > 0 && ABS_ERROR_SAMPLES <= TOLERANCE_SAMPLES)); then
    converged=1
    break
  fi
  sleep 0.5
done

if ((converged == 0)); then
  tail -100 "${GNB_LOG}" || true
  tail -100 "${UE_LOG}" || true
  die "TA loop did not follow the modeled delay"
fi

echo
echo "============================================================"
echo "TEST PASSED"
echo "Modeled delay increase: ${EXPECTED_SAMPLES} samples"
echo "UE TA correction:       ${APPLIED_SAMPLES} samples (${TA_COMMAND_COUNT} commands)"
echo "Tracking error:         ${ERROR_SAMPLES} samples"
echo "============================================================"
