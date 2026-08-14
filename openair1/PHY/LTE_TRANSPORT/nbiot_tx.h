/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef OAI_NBIOT_TX_H
#define OAI_NBIOT_TX_H

#include <stdbool.h>
#include <stdint.h>

#define NBIOT_MIB_BITS 34
#define NBIOT_NPBCH_CRC_BITS 16
#define NBIOT_NPBCH_D (NBIOT_MIB_BITS + NBIOT_NPBCH_CRC_BITS)
#define NBIOT_NPBCH_E 1600

typedef struct {
  bool initialized;
  bool encoded;
  uint16_t nid_cell;
  uint8_t scheduling_info_sib1;
  uint8_t system_info_value_tag;
  bool access_barring;
  uint8_t raster_offset;
  uint8_t mib[5];
  uint8_t npbch_d[96 + 3 * NBIOT_NPBCH_D];
  uint8_t npbch_w[3 * 3 * NBIOT_NPBCH_D];
  uint8_t npbch_e[NBIOT_NPBCH_E];
} nbiot_tx_state_t;

struct PHY_VARS_eNB_s;

int generate_nbiot_guardband(struct PHY_VARS_eNB_s *eNB,
                             int32_t **txdataF,
                             int16_t amp,
                             uint16_t frame,
                             uint16_t subframe);

bool nbiot_guardband_reserves_subframe(uint16_t frame, uint16_t subframe);

#endif /* OAI_NBIOT_TX_H */
