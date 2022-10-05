/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#ifndef PDU_SESSION_ESTABLISHMENT_ACCEPT_H_
#define PDU_SESSION_ESTABLISHMENT_ACCEPT_H_

#include <stdint.h>

/* PDU Session Establish Accept Optional IE Identifiers - TS 24.501 Table 8.3.2.1.1 */

#define IEI_5GSM_CAUSE      0x59 /* 5GSM cause 9.11.4.2  */
#define IEI_PDU_ADDRESS     0x29 /* PDU address 9.11.4.10 */
#define IEI_RQ_TIMER_VALUE  0x56 /* GPRS timer 9.11.2.3  */
#define IEI_SNSSAI          0x22 /* S-NSSAI 9.11.2.8  */
#define IEI_ALWAYSON_PDU    0x80 /* Always-on PDU session indication 9.11.4.3 */
#define IEI_MAPPED_EPS      0x75 /* Mapped EPS bearer contexts 9.11.4.8  */
#define IEI_EAP_MSG         0x78 /* EAP message 9.11.2.2  */
#define IEI_AUTH_QOS_DESC   0x79 /* QoS flow descriptions 9.11.4.12 */
#define IEI_EXT_CONF_OPT    0x7b /* Extended protocol configuration options 9.11.4.6  */
#define IEI_DNN             0x25 /* DNN 9.11.2.1B  */

/* PDU Session type value - TS 24.501 Table 9.11.4.10.1*/

#define PDU_SESSION_TYPE_IPV4   0b001
#define PDU_SESSION_TYPE_IPV6   0b010
#define PDU_SESSION_TYPE_IPV4V6 0b011

/* Rule operation codes - TS 24.501 Table 9.11.4.13.1 */

#define ROC_RESERVED_0                  0b000 /* Reserved */
#define ROC_CREATE_NEW_QOS_RULE         0b001 /* Create new QoS rule */
#define ROC_DELETE_QOS_RULE             0b010 /* Delete existing QoS rule */
#define ROC_MODIFY_QOS_RULE_ADD_PF      0b011 /* Modify existing QoS rule and add packet filters */
#define ROC_MODIFY_QOS_RULE_REPLACE_PF  0b100 /* Modify existing QoS rule and replace all packet filters */
#define ROC_MODIFY_QOS_RULE_DELETE_PF   0b101 /* Modify existing QoS rule and delete packet filters */
#define ROC_MODIFY_QOS_RULE_WITHOUT_PF  0b110 /* Modify existing QoS rule without modifying packet filters */
#define ROC_RESERVED_1                  0b111 /* Reserved */

/* DNN - ASCII Codes */

#define ASCII_ACK 0x06 /* Delimiter in the DNN IEI */

/* Mandatory Presence IE - TS 24.501 Table 8.3.2.1.1 */

typedef struct packet_filter_create_qos_rule_s {
  uint8_t pf_dir; /* Packet filter direction */
  uint8_t pf_id;  /* Packet filter identifier */
  uint8_t length; /* Length of packet filter contents */
} packet_filter_type1_t; /* TS 24.501 Figure 9.11.4.13.3 */

typedef struct packet_filter_modify_qos_rule_s {
  uint8_t pf_id;  /* Packet filter identifier */
} packet_filter_type2_t; /* TS 24.501 Figure 9.11.4.13.4 */

typedef struct packet_filter_s {
  union pf_type {
    packet_filter_type1_t type_1;
    packet_filter_type2_t type_2;
  } pf_type;
} packet_filter_t;

typedef struct qos_rule_s {
  uint8_t  id;      /* QoS rule identifier */
  uint16_t length;  /* Length of QoS Rule */
  uint8_t  oc;      /* Rule operation code (3bits) */
  uint8_t  dqr;     /* DQR bit (1 bit) */
  uint8_t  nb_pf;   /* Number of packet filters (4 bits) */
  uint8_t  prcd;    /* QoS rule precedence */
  uint8_t  qfi;     /* QoS Flow Identifier */
} qos_rule_t;

typedef struct auth_qos_rules_s {
  uint16_t length;  /* Length of QoS rules IE */
} auth_qos_rule_t;  /* QoS Rule as defined in 24.501 Figure 9.11.4.13.2 */

typedef struct session_ambr_s {
  uint8_t  length;  /* Length of Session-AMBR contents */
  uint8_t  unit_dl; /* Unit for Session-AMBR for downlink */
  uint16_t sess_dl; /* Session-AMBR for downlink */
  uint8_t  unit_ul; /* Unit for Session-AMBR for uplink */
  uint16_t sess_ul; /* Session-AMBR for uplink */
} session_ambr_t;   /* TS 24.501 Figure 9.11.4.14.1 */

#endif