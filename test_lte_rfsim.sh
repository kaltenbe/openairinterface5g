#!/usr/bin/env bash
set -euo pipefail

# ----------------------------------------------------------------------
# OAI LTE eNB / UE RFsim integration test
# ----------------------------------------------------------------------

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
OAI_BIN_DIR="${OAI_BIN_DIR:-${SCRIPT_DIR}/cmake_targets/ran_build/build}"

ENB_BIN="./lte-softmodem"
UE_BIN="./lte-uesoftmodem"
ENB_CONFIG="../../../targets/PROJECTS/GENERIC-LTE-EPC/CONF/enb.band7.tm1.50PRB.usrpb210.conf"
CFO_CONFIG="../../../lte_rfsim_cfo.conf"
UE_CFO_HZ="${UE_CFO_HZ:-10000}"
SIM_CFO_HZ="${SIM_CFO_HZ:-10000}"

ENB_ARGS=(
    -O "${ENB_CONFIG}"
    --rfsim
    '--rfsimulator.[0].serverport' 4044
)

UE_ARGS=(
    -C 2685000000
    -r 50
    --rfsim
    --rfsimulator.serverport 4044
)

if ((UE_CFO_HZ != 0)); then
    UE_ARGS+=(--ue-cfo "${UE_CFO_HZ}")
fi

if ((SIM_CFO_HZ != 0)); then
    UE_ARGS+=(
        -O "${CFO_CONFIG}"
        --rfsimulator.options chanmod
        '--channelmod.cfo_models.[0].cfo_hz' "${SIM_CFO_HZ}"
    )
fi

LOG_DIR="${LOG_DIR:-${SCRIPT_DIR}/test-logs/lte-rfsim}"
ENB_LOG="${LOG_DIR}/enb.log"
UE_LOG="${LOG_DIR}/ue.log"

ENB_STARTUP_DELAY="${ENB_STARTUP_DELAY:-2}"
TEST_TIMEOUT="${TEST_TIMEOUT:-30}"

UE_SUCCESS_PATTERN="Logical Channel UL-DCCH (SRB1), Generating RRCConnectionSetupComplete"
ENB_SUCCESS_PATTERN="Logical Channel UL-DCCH, processing LTE_RRCConnectionSetupComplete from UE (SRB1 Active)"

ENB_PID=""
UE_PID=""

mkdir -p "${LOG_DIR}"
: > "${ENB_LOG}"
: > "${UE_LOG}"

if [[ ! -d "${OAI_BIN_DIR}" ]]; then
    echo "ERROR: build directory not found:"
    echo "  ${OAI_BIN_DIR}"
    exit 1
fi

cd "${OAI_BIN_DIR}"

if [[ ! -x "${ENB_BIN}" ]]; then
    echo "ERROR: eNB executable not found or not executable:"
    echo "  ${OAI_BIN_DIR}/${ENB_BIN#./}"
    exit 1
fi

if [[ ! -x "${UE_BIN}" ]]; then
    echo "ERROR: UE executable not found or not executable:"
    echo "  ${OAI_BIN_DIR}/${UE_BIN#./}"
    exit 1
fi

if [[ ! -f "${ENB_CONFIG}" ]]; then
    echo "ERROR: eNB configuration file not found:"
    echo "  ${ENB_CONFIG}"
    exit 1
fi

if ((SIM_CFO_HZ != 0)) && [[ ! -f "${CFO_CONFIG}" ]]; then
    echo "ERROR: CFO channel-model configuration file not found:"
    echo "  ${CFO_CONFIG}"
    exit 1
fi

cleanup() {
    local rc=$?

    trap - EXIT INT TERM

    echo
    echo "Stopping OAI processes..."

    if [[ -n "${UE_PID}" ]] && kill -0 "${UE_PID}" 2>/dev/null; then
        kill -INT "${UE_PID}" 2>/dev/null || true
    fi

    if [[ -n "${ENB_PID}" ]] && kill -0 "${ENB_PID}" 2>/dev/null; then
        kill -INT "${ENB_PID}" 2>/dev/null || true
    fi

    sleep 2

    if [[ -n "${UE_PID}" ]] && kill -0 "${UE_PID}" 2>/dev/null; then
        kill -TERM "${UE_PID}" 2>/dev/null || true
    fi

    if [[ -n "${ENB_PID}" ]] && kill -0 "${ENB_PID}" 2>/dev/null; then
        kill -TERM "${ENB_PID}" 2>/dev/null || true
    fi

    [[ -z "${UE_PID}" ]] || wait "${UE_PID}" 2>/dev/null || true
    [[ -z "${ENB_PID}" ]] || wait "${ENB_PID}" 2>/dev/null || true

    echo "Logs:"
    echo "  eNB: ${ENB_LOG}"
    echo "  UE:  ${UE_LOG}"

    exit "${rc}"
}

trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

echo "Starting LTE eNB:"
printf '  %q' "${ENB_BIN}" "${ENB_ARGS[@]}"
echo

stdbuf -oL -eL \
    "${ENB_BIN}" "${ENB_ARGS[@]}" \
    > >(sed -u 's/^/[eNB] /' | tee "${ENB_LOG}") 2>&1 &
ENB_PID=$!

sleep "${ENB_STARTUP_DELAY}"

if ! kill -0 "${ENB_PID}" 2>/dev/null; then
    echo "ERROR: eNB terminated before the UE was started."
    tail -50 "${ENB_LOG}" || true
    exit 1
fi

echo
echo "Starting LTE UE:"
printf '  %q' "${UE_BIN}" "${UE_ARGS[@]}"
echo

stdbuf -oL -eL \
    "${UE_BIN}" "${UE_ARGS[@]}" \
    > >(sed -u 's/^/[UE] /' | tee "${UE_LOG}") 2>&1 &
UE_PID=$!

echo
echo "Waiting up to ${TEST_TIMEOUT} seconds for LTE RRC setup to complete..."

for ((i = 0; i < TEST_TIMEOUT * 10; i++)); do
    ue_succeeded=0
    enb_succeeded=0

    grep -Fq -- "${UE_SUCCESS_PATTERN}" "${UE_LOG}" && ue_succeeded=1
    grep -Fq -- "${ENB_SUCCESS_PATTERN}" "${ENB_LOG}" && enb_succeeded=1

    if ((ue_succeeded && enb_succeeded)); then
        echo
        echo "============================================================"
        echo "TEST PASSED"
        echo "============================================================"
        echo "UE and eNB both logged RRCConnectionSetupComplete."
        exit 0
    fi

    if ! kill -0 "${ENB_PID}" 2>/dev/null; then
        echo "ERROR: eNB terminated before the success criteria were met."
        tail -50 "${ENB_LOG}" || true
        exit 1
    fi

    if ! kill -0 "${UE_PID}" 2>/dev/null; then
        echo "ERROR: UE terminated before the success criteria were met."
        tail -50 "${UE_LOG}" || true
        exit 1
    fi

    sleep 0.1
done

echo "ERROR: success criteria were not met within ${TEST_TIMEOUT} seconds."

if ! grep -Fq -- "${UE_SUCCESS_PATTERN}" "${UE_LOG}"; then
    echo "Missing UE log message:"
    echo "  ${UE_SUCCESS_PATTERN}"
fi

if ! grep -Fq -- "${ENB_SUCCESS_PATTERN}" "${ENB_LOG}"; then
    echo "Missing eNB log message:"
    echo "  ${ENB_SUCCESS_PATTERN}"
fi

echo
echo "Last 50 lines of eNB log:"
tail -50 "${ENB_LOG}" || true
echo
echo "Last 50 lines of UE log:"
tail -50 "${UE_LOG}" || true
exit 1
