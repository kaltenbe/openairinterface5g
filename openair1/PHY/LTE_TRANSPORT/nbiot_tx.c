/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * Minimal NB-IoT anchor-channel transmitter for an NB carrier placed in the
 * guardband PRB immediately below LTE PRB 0.
 *
 * Sequence generation and RE mapping follow 3GPP TS 36.211 clauses 10.2.4,
 * 10.2.6 and 10.2.7. NPBCH coding follows TS 36.212 clause 6.4.1.
 *
 * This module produces the NB resource grid. The 7.5 kHz half-subcarrier
 * shift required for guardband operation belongs in a separate OFDM overlay;
 * the legacy LTE RU path does not apply it to these REs yet.
 */

#include "PHY/LTE_TRANSPORT/nbiot_tx.h"

#include "PHY/CODING/coding_defs.h"
#include "PHY/defs_eNB.h"
#include "PHY/gold.h"
#include "common/utils/LOG/log.h"

#include <math.h>
#include <string.h>

#define NBIOT_NSC 12
#define NBIOT_NPSS_NSC 11
#define NBIOT_SYNC_SYMBOL_START 3
#define NBIOT_NPBCH_RE 100
#define NBIOT_NPBCH_BITS_PER_BLOCK (2 * NBIOT_NPBCH_RE)
#define NBIOT_NPBCH_BLOCKS 8
#define NBIOT_NPBCH_REPETITIONS 8
#define Q15_ONE_OVER_SQRT2 23170

static const int8_t npss_symbol_sign[11] = {1, 1, 1, 1, -1, -1, 1, 1, 1, -1, 1};

/* TS 36.211 Table 10.2.7.2.1-1. */
static const int8_t nsss_bqm[4][128] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1,
     1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1,
     1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1,
     1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1},
    {1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1,
     -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1,
     -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1,
     -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1,
     1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1},
    {1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1,
     -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1,
     -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1,
     1, -1, 1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1,
     -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1}};

static int32_t pack_complex(int16_t re, int16_t im)
{
  union {
    int32_t word;
    int16_t component[2];
  } sample = {.component = {re, im}};
  return sample.word;
}

static int16_t scale_component(int16_t amp, double value)
{
  long scaled = lrint((double)amp * value);
  if (scaled > INT16_MAX)
    scaled = INT16_MAX;
  else if (scaled < INT16_MIN)
    scaled = INT16_MIN;
  return (int16_t)scaled;
}

static uint32_t guardband_re(const LTE_DL_FRAME_PARMS *fp, uint16_t subframe, uint16_t symbol, uint16_t k)
{
  const uint32_t subframe_offset = subframe * fp->symbols_per_tti * fp->ofdm_symbol_size;
  return subframe_offset + symbol * fp->ofdm_symbol_size + fp->first_carrier_offset - NBIOT_NSC + k;
}

static int validate_grid(const LTE_DL_FRAME_PARMS *fp, int32_t **txdataF, uint16_t subframe)
{
  if (txdataF == NULL || txdataF[0] == NULL || subframe >= LTE_NUMBER_OF_SUBFRAMES_PER_FRAME)
    return -1;
  if (fp->Ncp != NORMAL || fp->symbols_per_tti != 14) {
    LOG_E(PHY, "NB-IoT guardband TX currently requires normal cyclic prefix\n");
    return -1;
  }
  if (fp->first_carrier_offset < NBIOT_NSC) {
    LOG_E(PHY, "NB-IoT guardband TX has no PRB below first carrier offset %u\n", fp->first_carrier_offset);
    return -1;
  }
  return 0;
}

bool nbiot_guardband_reserves_subframe(uint16_t frame, uint16_t subframe)
{
  return subframe == 0 || subframe == 5 || (subframe == 9 && !(frame & 1));
}

static void generate_npss(const LTE_DL_FRAME_PARMS *fp, int32_t **txdataF, int16_t amp, uint16_t subframe)
{
  for (uint16_t l = 0; l < 11; l++) {
    for (uint16_t n = 0; n < NBIOT_NPSS_NSC; n++) {
      const double phase = -M_PI * 5.0 * n * (n + 1) / 11.0;
      const double sign = npss_symbol_sign[l];
      const int16_t re = scale_component(amp, sign * cos(phase));
      const int16_t im = scale_component(amp, sign * sin(phase));
      txdataF[0][guardband_re(fp, subframe, NBIOT_SYNC_SYMBOL_START + l, n)] = pack_complex(re, im);
    }
  }
}

static void generate_nsss(const LTE_DL_FRAME_PARMS *fp,
                          int32_t **txdataF,
                          int16_t amp,
                          uint16_t frame,
                          uint16_t subframe,
                          uint16_t nid_cell)
{
  const uint16_t u = (nid_cell % 126) + 3;
  const uint16_t q = nid_cell / 126;
  const double theta = ((frame >> 1) & 3) / 4.0;

  for (uint16_t n = 0; n < 132; n++) {
    const uint16_t nprime = n % 131;
    const uint16_t m = n % 128;
    const double phase = -2.0 * M_PI * theta * n - M_PI * u * nprime * (nprime + 1) / 131.0;
    const double sign = nsss_bqm[q][m];
    const int16_t re = scale_component(amp, sign * cos(phase));
    const int16_t im = scale_component(amp, sign * sin(phase));
    const uint16_t l = NBIOT_SYNC_SYMBOL_START + n / NBIOT_NSC;
    const uint16_t k = n % NBIOT_NSC;
    txdataF[0][guardband_re(fp, subframe, l, k)] = pack_complex(re, im);
  }
}

static uint32_t gold_word(uint32_t cinit, uint16_t word_index)
{
  uint32_t x1 = 0;
  uint32_t x2 = cinit;
  uint32_t word = 0;
  for (uint16_t i = 0; i <= word_index; i++)
    word = gold_generic(&x1, &x2, i == 0);
  return word;
}

static void generate_nrs(const LTE_DL_FRAME_PARMS *fp,
                         int32_t **txdataF,
                         int16_t amp,
                         uint16_t subframe,
                         uint16_t nid_cell)
{
  const int16_t qpsk_amp = ((int32_t)amp * Q15_ONE_OVER_SQRT2) >> 15;
  for (uint16_t slot = 0; slot < 2; slot++) {
    const uint16_t ns = (subframe << 1) + slot;
    for (uint16_t ref = 0; ref < 2; ref++) {
      const uint16_t l_slot = 5 + ref;
      const uint16_t l = slot * 7 + l_slot;
      const uint32_t cinit = 1024U * (7 * (ns + 1) + l_slot + 1) * (2 * nid_cell + 1) + 2 * nid_cell + 1;
      const uint32_t gold = gold_word(cinit, 6);
      const uint16_t v = ref ? 3 : 0;
      for (uint16_t m = 0; m < 2; m++) {
        const uint16_t mprime = m + 109;
        const uint8_t b0 = (gold >> (2 * (mprime & 0xf))) & 1;
        const uint8_t b1 = (gold >> (2 * (mprime & 0xf) + 1)) & 1;
        const uint16_t k = 6 * m + ((v + nid_cell) % 6);
        txdataF[0][guardband_re(fp, subframe, l, k)] =
            pack_complex(b0 ? -qpsk_amp : qpsk_amp, b1 ? -qpsk_amp : qpsk_amp);
      }
    }
  }
}

static void put_bits(uint8_t *payload, uint16_t *offset, uint32_t value, uint8_t width)
{
  for (uint8_t i = 0; i < width; i++, (*offset)++) {
    const uint8_t bit = (value >> (width - i - 1)) & 1;
    payload[*offset >> 3] |= bit << (7 - (*offset & 7));
  }
}

static void pack_mib(nbiot_tx_state_t *state, uint16_t frame)
{
  uint16_t offset = 0;
  memset(state->mib, 0, sizeof(state->mib));
  put_bits(state->mib, &offset, (frame >> 6) & 0xf, 4);
  put_bits(state->mib, &offset, 0, 2); /* hyperSFN-LSB */
  put_bits(state->mib, &offset, state->scheduling_info_sib1, 4);
  put_bits(state->mib, &offset, state->system_info_value_tag, 5);
  put_bits(state->mib, &offset, state->access_barring, 1);
  put_bits(state->mib, &offset, 2, 2); /* operationModeInfo: guardband */
  put_bits(state->mib, &offset, state->raster_offset, 2);
  put_bits(state->mib, &offset, 0, 3); /* Guardband-NB-r13 spare */
  put_bits(state->mib, &offset, 0, 1); /* additionalTransmissionSIB1-r15 */
  put_bits(state->mib, &offset, 0, 1); /* ab-Enabled-5GC-r16 */
  put_bits(state->mib, &offset, 0, 9);
}

static void scramble_npbch(nbiot_tx_state_t *state)
{
  uint32_t x1 = 0;
  uint32_t x2 = state->nid_cell;
  uint32_t word = 0;
  for (uint16_t i = 0; i < NBIOT_NPBCH_E; i++) {
    if (!(i & 31))
      word = gold_generic(&x1, &x2, i == 0);
    state->npbch_e[i] = (state->npbch_e[i] & 1) ^ ((word >> (i & 31)) & 1);
  }
}

static void encode_npbch(nbiot_tx_state_t *state, uint16_t frame)
{
  pack_mib(state, frame);
  ccodelte_encode(NBIOT_MIB_BITS, 2, state->mib, state->npbch_d + 96, 0);
  const uint32_t rcc = sub_block_interleaving_cc(NBIOT_NPBCH_D, state->npbch_d + 96, state->npbch_w);
  lte_rate_matching_cc(rcc, NBIOT_NPBCH_E, state->npbch_w, state->npbch_e);
  scramble_npbch(state);
  state->encoded = true;
}

static bool npbch_reference_re(uint16_t nid_cell, uint16_t l, uint16_t k)
{
  if (l == 3 || l == 9 || l == 10)
    return false;
  return (k % 3) == (nid_cell % 3);
}

static uint8_t npbch_phase(uint16_t nid_cell, uint16_t frame, uint16_t symbol_index)
{
  const uint32_t n = (frame & 7) + 1;
  const uint32_t cinit = 512U * (nid_cell + 1) * n * n * n + nid_cell;
  const uint16_t bit = 2 * symbol_index;
  const uint32_t word = gold_word(cinit, bit >> 5);
  return (word >> (bit & 31)) & 3;
}

static int32_t rotate_quarter(int16_t re, int16_t im, uint8_t phase)
{
  switch (phase) {
    case 1:
      return pack_complex(-im, re);
    case 2:
      return pack_complex(-re, -im);
    case 3:
      return pack_complex(im, -re);
    default:
      return pack_complex(re, im);
  }
}

static int generate_npbch(PHY_VARS_eNB *eNB,
                          int32_t **txdataF,
                          int16_t amp,
                          uint16_t frame,
                          uint16_t subframe)
{
  nbiot_tx_state_t *state = &eNB->nbiot_tx;
  const LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;
  if (!state->initialized) {
    state->nid_cell = fp->Nid_cell;
    state->scheduling_info_sib1 = 11;
    state->system_info_value_tag = 0;
    state->access_barring = false;
    state->raster_offset = 0;
    state->initialized = true;
  }
  if (state->nid_cell != fp->Nid_cell) {
    state->nid_cell = fp->Nid_cell;
    state->encoded = false;
  }
  if (!state->encoded || !(frame & 63))
    encode_npbch(state, frame);

  const uint16_t block = (frame / NBIOT_NPBCH_REPETITIONS) % NBIOT_NPBCH_BLOCKS;
  const uint16_t bit_start = block * NBIOT_NPBCH_BITS_PER_BLOCK;
  const int16_t qpsk_amp = ((int32_t)amp * Q15_ONE_OVER_SQRT2) >> 15;
  uint16_t symbol_index = 0;

  for (uint16_t l = NBIOT_SYNC_SYMBOL_START; l < 14; l++) {
    for (uint16_t k = 0; k < NBIOT_NSC; k++) {
      if (npbch_reference_re(state->nid_cell, l, k))
        continue;
      const uint16_t bit = bit_start + 2 * symbol_index;
      const int16_t re = state->npbch_e[bit] ? -qpsk_amp : qpsk_amp;
      const int16_t im = state->npbch_e[bit + 1] ? -qpsk_amp : qpsk_amp;
      const uint8_t phase = npbch_phase(state->nid_cell, frame, symbol_index);
      txdataF[0][guardband_re(fp, subframe, l, k)] = rotate_quarter(re, im, phase);
      symbol_index++;
    }
  }

  if (symbol_index != NBIOT_NPBCH_RE) {
    LOG_E(PHY, "NPBCH mapped %u RE instead of %u\n", symbol_index, NBIOT_NPBCH_RE);
    return -1;
  }
  return 0;
}

int generate_nbiot_guardband(PHY_VARS_eNB *eNB,
                             int32_t **txdataF,
                             int16_t amp,
                             uint16_t frame,
                             uint16_t subframe)
{
  LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;
  if (validate_grid(fp, txdataF, subframe) < 0 || fp->Nid_cell >= 504)
    return -1;

  if (subframe == 0) {
    if (generate_npbch(eNB, txdataF, amp, frame, subframe) < 0)
      return -1;
    generate_nrs(fp, txdataF, amp, subframe, fp->Nid_cell);
  } else if (subframe == 5) {
    generate_npss(fp, txdataF, amp, subframe);
  } else if (subframe == 9 && !(frame & 1)) {
    generate_nsss(fp, txdataF, amp, frame, subframe, fp->Nid_cell);
  }

  return 0;
}
