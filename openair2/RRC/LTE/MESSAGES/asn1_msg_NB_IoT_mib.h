/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef OAI_ASN1_MSG_NB_IOT_MIB_H
#define OAI_ASN1_MSG_NB_IOT_MIB_H

#include <stddef.h>
#include <stdint.h>

uint8_t do_MIB_NB_IoT_to_buffer(uint8_t *buffer,
                                size_t buffer_size,
                                uint32_t N_RB_DL,
                                uint32_t frame,
                                uint32_t hyper_frame);

#endif /* OAI_ASN1_MSG_NB_IOT_MIB_H */
