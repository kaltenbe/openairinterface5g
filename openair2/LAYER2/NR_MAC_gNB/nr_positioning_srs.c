/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
#include "nr_positioning_srs.h"
#include <string.h>

bool nr_positioning_srs_to_pdu(const f1ap_srs_resource_t *r, nfapi_nr_srs_pdu_t *pdu)
{
  static const uint16_t periods[] = {1, 2, 4, 5, 8, 10, 16, 20, 32, 40, 64, 80, 160, 320, 640, 1280, 2560};
  if (!r || !pdu || r->resource_type.present != F1AP_RESOURCE_TYPE_PR_PERIODIC
      || (unsigned)r->resource_type.choice.periodic.periodicity >= sizeof(periods) / sizeof(periods[0])
      || (unsigned)r->nr_of_srs_ports > F1AP_SRS_NUMBER_OF_PORTS_N4 || (unsigned)r->nr_of_symbols > F1AP_SRS_NUMBER_OF_SYMBOLS_N4
      || (unsigned)r->repetition_factor > F1AP_SRS_REPETITION_FACTOR_RF4 || (int)r->repetition_factor > (int)r->nr_of_symbols
      || r->start_position > 13 || (1U << r->nr_of_symbols) > r->start_position + 1U
      || r->c_srs > 63 || r->b_srs > 3 || r->b_hop > 3 || r->sequence_id > 1023
      || r->freq_domain_position > 67 || r->freq_domain_shift > 268
      || (unsigned)r->group_or_sequence_hopping > F1AP_GROUPORSEQUENCEHOPPING_SEQUENCEHOPPING)
    return false;

  const uint16_t period = periods[r->resource_type.choice.periodic.periodicity];
  if (r->resource_type.choice.periodic.offset >= period)
    return false;

  uint8_t comb_size, comb_offset, cyclic_shift;
  switch (r->transmission_comb.present) {
    case F1AP_TRANSMISSION_COMB_PR_N2:
      comb_size = 0;
      comb_offset = r->transmission_comb.choice.n2.comb_offset_n2;
      cyclic_shift = r->transmission_comb.choice.n2.cyclic_shift_n2;
      if (comb_offset > 1 || cyclic_shift > 7)
        return false;
      break;
    case F1AP_TRANSMISSION_COMB_PR_N4:
      comb_size = 1;
      comb_offset = r->transmission_comb.choice.n4.comb_offset_n4;
      cyclic_shift = r->transmission_comb.choice.n4.cyclic_shift_n4;
      if (comb_offset > 3 || cyclic_shift > 11)
        return false;
      break;
    default:
      return false;
  }

  memset(pdu, 0, sizeof(*pdu));
  pdu->num_ant_ports = r->nr_of_srs_ports;
  pdu->num_symbols = r->nr_of_symbols;
  pdu->num_repetitions = r->repetition_factor;
  pdu->time_start_position = 13 - r->start_position;
  pdu->config_index = r->c_srs;
  pdu->sequence_id = r->sequence_id;
  pdu->bandwidth_index = r->b_srs;
  pdu->comb_size = comb_size;
  pdu->comb_offset = comb_offset;
  pdu->cyclic_shift = cyclic_shift;
  pdu->frequency_position = r->freq_domain_position;
  pdu->frequency_shift = r->freq_domain_shift;
  pdu->frequency_hopping = r->b_hop;
  pdu->group_or_sequence_hopping = r->group_or_sequence_hopping;
  pdu->resource_type = 2;
  pdu->t_srs = period;
  pdu->t_offset = r->resource_type.choice.periodic.offset;
  return true;
}

bool nr_same_positioning_srs(const nfapi_nr_srs_pdu_t *a, const nfapi_nr_srs_pdu_t *b)
{
  return a->bwp_start == b->bwp_start && a->bwp_size == b->bwp_size && a->subcarrier_spacing == b->subcarrier_spacing
         && a->cyclic_prefix == b->cyclic_prefix && a->num_ant_ports == b->num_ant_ports && a->num_symbols == b->num_symbols
         && a->num_repetitions == b->num_repetitions && a->time_start_position == b->time_start_position
         && a->config_index == b->config_index && a->sequence_id == b->sequence_id && a->bandwidth_index == b->bandwidth_index
         && a->comb_size == b->comb_size && a->comb_offset == b->comb_offset && a->cyclic_shift == b->cyclic_shift
         && a->frequency_position == b->frequency_position && a->frequency_shift == b->frequency_shift
         && a->frequency_hopping == b->frequency_hopping && a->group_or_sequence_hopping == b->group_or_sequence_hopping
         && a->resource_type == b->resource_type && a->t_srs == b->t_srs && a->t_offset == b->t_offset;
}
