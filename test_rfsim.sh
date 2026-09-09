#!/usr/bin/env bash
set -euo pipefail

# ----------------------------------------------------------------------
# OAI gNB / UE RFsim integration test
# ----------------------------------------------------------------------

# Directory containing nr-softmodem and nr-uesoftmodem
OAI_BIN_DIR="${OAI_BIN_DIR:-cmake_targets/ran_build/build}"

GNB_BIN="${OAI_BIN_DIR}/nr-softmodem"
UE_BIN="${OAI_BIN_DIR}/nr-uesoftmodem"

# gNB configuration
GNB_CONFIG="${GNB_CONFIG:-gnb.sa.band254.u0.25prb.rfsim.ntn-leo-RegenWithPRS.conf}"
UE_CONFIG="${UE_CONFIG:-ue_Leo_Regen.conf}"
PRS_INPUT="${PRS_INPUT:-config}"
UE_POS_SIB_CONFIG="${UE_POS_SIB_CONFIG:-ue_Leo_Regen_possib.conf}"

case "${PRS_INPUT}" in
    config) ;;
    possib) UE_CONFIG="${UE_POS_SIB_CONFIG}" ;;
    *)
        echo "ERROR: PRS_INPUT must be 'config' or 'possib' (got '${PRS_INPUT}')."
        exit 1
        ;;
esac

# Default UE command-line parameters
UE_ARGS=(
    -C 2488400000
    --CO -873500000
    -r 25
    --numerology 0
    --ssb 60
    --rfsim
    -O "${UE_CONFIG}"
    #--log_config.ASN1_debug 1
    #--log_config.nr_rrc_log_level debug
)

# Logs
LOG_DIR="${LOG_DIR:-./test-logs}"
GNB_LOG="${LOG_DIR}/gnb.log"
UE_LOG="${LOG_DIR}/ue.log"

# Timeouts
GNB_STARTUP_TIMEOUT="${GNB_STARTUP_TIMEOUT:-30}"
TEST_DURATION="${TEST_DURATION:-30}"

# This exact message means that the gNB is ready
GNB_READY_PATTERN="Command line parameters for OAI UE"
UE_PRS_PATTERN="${UE_PRS_PATTERN:-DL PRS ToA}"
UE_POS_SIB_PATTERN="${UE_POS_SIB_PATTERN:-PosSIB configured}"

GNB_PID=""
UE_PID=""

# ----------------------------------------------------------------------
# Initial checks
# ----------------------------------------------------------------------

mkdir -p "${LOG_DIR}"

: > "${GNB_LOG}"
: > "${UE_LOG}"

if [[ ! -x "${GNB_BIN}" ]]; then
    echo "ERROR: gNB executable not found or not executable:"
    echo "  ${GNB_BIN}"
    exit 1
fi

if [[ ! -x "${UE_BIN}" ]]; then
    echo "ERROR: UE executable not found or not executable:"
    echo "  ${UE_BIN}"
    exit 1
fi

if [[ ! -f "${GNB_CONFIG}" ]]; then
    echo "ERROR: gNB configuration file not found:"
    echo "  ${GNB_CONFIG}"
    exit 1
fi

if [[ ! -f "${UE_CONFIG}" ]]; then
    echo "ERROR: UE configuration file not found:"
    echo "  ${UE_CONFIG}"
    exit 1
fi

# ----------------------------------------------------------------------
# Cleanup
# ----------------------------------------------------------------------

cleanup() {
    rc=$?

    trap - EXIT INT TERM

    echo
    echo "Stopping OAI processes..."

    if [[ -n "${UE_PID}" ]] && kill -0 "${UE_PID}" 2>/dev/null; then
        echo "Stopping UE (PID ${UE_PID})..."
        kill -INT "${UE_PID}" 2>/dev/null || true
    fi

    if [[ -n "${GNB_PID}" ]] && kill -0 "${GNB_PID}" 2>/dev/null; then
        echo "Stopping gNB (PID ${GNB_PID})..."
        kill -INT "${GNB_PID}" 2>/dev/null || true
    fi

    # Give OAI a chance to shut down cleanly
    sleep 2

    if [[ -n "${UE_PID}" ]] && kill -0 "${UE_PID}" 2>/dev/null; then
        echo "UE still running, sending SIGTERM..."
        kill -TERM "${UE_PID}" 2>/dev/null || true
    fi

    if [[ -n "${GNB_PID}" ]] && kill -0 "${GNB_PID}" 2>/dev/null; then
        echo "gNB still running, sending SIGTERM..."
        kill -TERM "${GNB_PID}" 2>/dev/null || true
    fi

    wait "${UE_PID}" 2>/dev/null || true
    wait "${GNB_PID}" 2>/dev/null || true

    echo
    echo "Logs:"
    echo "  gNB: ${GNB_LOG}"
    echo "  UE:  ${UE_LOG}"

    return "${rc}"
}

trap cleanup EXIT INT TERM

# ----------------------------------------------------------------------
# Start gNB
# ----------------------------------------------------------------------

echo "============================================================"
echo "Starting gNB"
echo "============================================================"

echo "Executable:"
echo "  ${GNB_BIN}"

echo "Configuration:"
echo "  ${GNB_CONFIG}"

echo "Log:"
echo "  ${GNB_LOG}"

echo

stdbuf -oL -eL \
    "${GNB_BIN}" \
    -O "${GNB_CONFIG}" \
    --rfsim \
    > >(sed -u 's/^/[gNB] /' | tee "${GNB_LOG}") 2>&1 &

GNB_PID=$!

echo
echo "gNB PID: ${GNB_PID}"

# ----------------------------------------------------------------------
# Wait for gNB readiness
# ----------------------------------------------------------------------

echo
echo "Waiting for gNB readiness message:"
echo "  ${GNB_READY_PATTERN}"
echo

gnb_ready=0

for ((i = 0; i < GNB_STARTUP_TIMEOUT * 10; i++)); do

    if grep -Fq "${GNB_READY_PATTERN}" "${GNB_LOG}"; then
        gnb_ready=1
        break
    fi

    if ! kill -0 "${GNB_PID}" 2>/dev/null; then
        echo
        echo "ERROR: gNB terminated before becoming ready."
        exit 1
    fi

    sleep 0.1
done

if [[ "${gnb_ready}" != "1" ]]; then
    echo
    echo "ERROR: gNB did not become ready within ${GNB_STARTUP_TIMEOUT} seconds."
    echo
    echo "Expected message:"
    echo "  ${GNB_READY_PATTERN}"
    echo
    echo "Last 50 lines of gNB log:"
    tail -50 "${GNB_LOG}" || true
    exit 1
fi

echo
echo "============================================================"
echo "gNB is ready"
echo "============================================================"

# ----------------------------------------------------------------------
# Start UE
# ----------------------------------------------------------------------

echo
echo "============================================================"
echo "Starting UE"
echo "============================================================"

echo "Executable:"
echo "  ${UE_BIN}"

echo "Command-line parameters:"
printf '  %q' "${UE_ARGS[@]}"
echo

echo "Log:"
echo "  ${UE_LOG}"

echo

stdbuf -oL -eL \
    "${UE_BIN}" \
    "${UE_ARGS[@]}" \
    > >(sed -u 's/^/[UE] /' | tee "${UE_LOG}") 2>&1 &

UE_PID=$!

echo
echo "UE PID: ${UE_PID}"

# ----------------------------------------------------------------------
# Run test
# ----------------------------------------------------------------------

echo
echo "============================================================"
echo "gNB and UE are running"
echo "Test duration: ${TEST_DURATION} seconds"
echo "============================================================"
echo

for ((i = 1; i <= TEST_DURATION; i++)); do

    if ! kill -0 "${GNB_PID}" 2>/dev/null; then
        echo
        echo "ERROR: gNB terminated unexpectedly."
        echo
        echo "Last 50 lines of gNB log:"
        tail -50 "${GNB_LOG}" || true
        exit 1
    fi

    if ! kill -0 "${UE_PID}" 2>/dev/null; then
        echo
        echo "ERROR: UE terminated unexpectedly."
        echo
        echo "Last 50 lines of UE log:"
        tail -50 "${UE_LOG}" || true
        exit 1
    fi

    sleep 1
done

# ----------------------------------------------------------------------
# Test result
# ----------------------------------------------------------------------

if ! grep -Fq "${UE_PRS_PATTERN}" "${UE_LOG}"; then
    echo
    echo "ERROR: UE did not report a PRS time-of-arrival measurement."
    echo
    echo "Expected message:"
    echo "  ${UE_PRS_PATTERN}"
    echo
    echo "PRS-related UE log messages:"
    grep -F "PRS" "${UE_LOG}" | tail -50 || true
    exit 1
fi

if [[ "${PRS_INPUT}" == "possib" ]] && ! grep -Fq "${UE_POS_SIB_PATTERN}" "${UE_LOG}"; then
    echo
    echo "ERROR: UE did not configure PRS from PosSIB."
    echo
    echo "Expected message:"
    echo "  ${UE_POS_SIB_PATTERN}"
    exit 1
fi

echo
echo "============================================================"
echo "TEST PASSED"
echo "============================================================"
