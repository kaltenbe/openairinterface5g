/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "PHY/CODING/coding_defs.h"
#include "PHY/LTE_TRANSPORT/nbiot_tx.h"
#include "PHY/defs_eNB.h"
#include "RRC/LTE/MESSAGES/asn1_msg_NB_IoT_mib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_FFT_SIZE 128
#define TEST_FIRST_CARRIER 64
#define TEST_SYMBOLS_PER_SUBFRAME 14
#define TEST_GUARDBAND_SC 12
#define TEST_AMPLITUDE 20000

static size_t count_guardband_re(const int32_t *grid, uint16_t subframe)
{
  size_t count = 0;
  const size_t sf = subframe * TEST_SYMBOLS_PER_SUBFRAME * TEST_FFT_SIZE;
  for (uint16_t l = 0; l < TEST_SYMBOLS_PER_SUBFRAME; ++l)
    for (uint16_t k = 0; k < TEST_GUARDBAND_SC; ++k)
      count += grid[sf + l * TEST_FFT_SIZE + TEST_FIRST_CARRIER - TEST_GUARDBAND_SC + k] != 0;
  return count;
}

static int expect_count(PHY_VARS_eNB *enb,
                        int32_t **antenna,
                        int32_t *grid,
                        uint16_t frame,
                        uint16_t subframe,
                        size_t expected)
{
  memset(grid,
         0,
         LTE_NUMBER_OF_SUBFRAMES_PER_FRAME * TEST_SYMBOLS_PER_SUBFRAME * TEST_FFT_SIZE * sizeof(*grid));
  if (generate_nbiot_guardband(enb, antenna, TEST_AMPLITUDE, frame, subframe) != 0) {
    fprintf(stderr, "generation failed for frame %u subframe %u\n", frame, subframe);
    return -1;
  }

  const size_t actual = count_guardband_re(grid, subframe);
  if (actual != expected) {
    fprintf(stderr,
            "frame %u subframe %u mapped %zu guardband RE, expected %zu\n",
            frame,
            subframe,
            actual,
            expected);
    return -1;
  }
  return 0;
}

int main(void)
{
  PHY_VARS_eNB *enb = calloc(1, sizeof(*enb));
  const size_t grid_size = LTE_NUMBER_OF_SUBFRAMES_PER_FRAME * TEST_SYMBOLS_PER_SUBFRAME * TEST_FFT_SIZE;
  int32_t *grid = calloc(grid_size, sizeof(*grid));
  if (enb == NULL || grid == NULL) {
    free(grid);
    free(enb);
    return EXIT_FAILURE;
  }

  LTE_DL_FRAME_PARMS *fp = &enb->frame_parms;
  fp->ofdm_symbol_size = TEST_FFT_SIZE;
  fp->first_carrier_offset = TEST_FIRST_CARRIER;
  fp->symbols_per_tti = TEST_SYMBOLS_PER_SUBFRAME;
  fp->Ncp = NORMAL;
  fp->Nid_cell = 42;
  int32_t *antenna[1] = {grid};

  ccodelte_init();
  crcTableInit();

  int failed = 0;
  uint8_t mib[5] = {0};
  const uint8_t expected_mib[5] = {0x02, 0xc0, 0x80, 0x00, 0x00};
  if (do_MIB_NB_IoT_to_buffer(mib, sizeof(mib), 50, 0, 0) != sizeof(mib) ||
      memcmp(mib, expected_mib, sizeof(mib)) != 0)
    failed = 1;

  failed |= expect_count(enb, antenna, grid, 0, 5, 121);
  failed |= expect_count(enb, antenna, grid, 0, 9, 132);
  failed |= expect_count(enb, antenna, grid, 1, 9, 0);
  failed |= expect_count(enb, antenna, grid, 0, 0, 108);

  for (uint16_t l = 0; l < 3; ++l)
    for (uint16_t k = 0; k < TEST_GUARDBAND_SC; ++k)
      failed |= grid[l * TEST_FFT_SIZE + TEST_FIRST_CARRIER - TEST_GUARDBAND_SC + k] != 0;

  failed |= !nbiot_guardband_reserves_subframe(0, 0);
  failed |= !nbiot_guardband_reserves_subframe(0, 5);
  failed |= !nbiot_guardband_reserves_subframe(0, 9);
  failed |= nbiot_guardband_reserves_subframe(1, 9);
  failed |= nbiot_guardband_reserves_subframe(0, 1);

  free(grid);
  free(enb);
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
