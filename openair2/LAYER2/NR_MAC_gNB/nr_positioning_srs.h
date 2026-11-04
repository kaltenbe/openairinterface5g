/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
#ifndef NR_POSITIONING_SRS_H
#define NR_POSITIONING_SRS_H

#include <stdbool.h>
#include "f1ap_messages_types.h"
#include "nfapi_nr_interface_scf.h"

#define NR_POSITIONING_SRS_RNTI 0xfffe

bool nr_same_positioning_srs(const nfapi_nr_srs_pdu_t *a, const nfapi_nr_srs_pdu_t *b);
bool nr_positioning_srs_to_pdu(const f1ap_srs_resource_t *resource, nfapi_nr_srs_pdu_t *pdu);

#endif
