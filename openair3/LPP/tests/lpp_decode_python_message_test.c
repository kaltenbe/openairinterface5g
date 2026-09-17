/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 *
 * Standalone test: decode a UPER-encoded LPP message (given as a hex
 * string) using OAI's asn1c-generated decoder, and print the result
 * as XER for inspection. Used to verify cross-tool (Python/Pycrate <-> C/asn1c)
 * LPP interoperability.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LPP_LPP-Message.h"
#include "per_decoder.h"
#include "xer_encoder.h"

// Paste the hex string produced by the Python/LMF side here:
static const char *PYTHON_ENCODED_HEX =
    "f003004620053d2c60b043100d800000000800f2c0840040200000000a000de82c57829b8b8404012602301ba02201b8808e1c0300c00021080037ba2080890a020b0008";

static void fail(const char *msg)
{
  fprintf(stderr, "FAIL: %s\n", msg);
  exit(EXIT_FAILURE);
}

// Converts a hex string like "f003..." into raw bytes.
static size_t hex_to_bytes(const char *hex, uint8_t *out, size_t out_size)
{
  size_t hex_len = strlen(hex);
  if (hex_len % 2 != 0)
    fail("hex string has odd length");
  size_t n_bytes = hex_len / 2;
  if (n_bytes > out_size)
    fail("hex string too long for buffer");
  for (size_t i = 0; i < n_bytes; i++) {
    unsigned int byte_val;
    if (sscanf(hex + 2 * i, "%2x", &byte_val) != 1)
      fail("invalid hex character");
    out[i] = (uint8_t)byte_val;
  }
  return n_bytes;
}

int main(void)
{
  uint8_t buffer[8192] = {0};
  size_t n_bytes = hex_to_bytes(PYTHON_ENCODED_HEX, buffer, sizeof(buffer));
  printf("Decoding %zu bytes from Python-generated hex...\n", n_bytes);

  LPP_LPP_Message_t *decoded = NULL;
  asn_codec_ctx_t ctx = {.max_stack_size = 100 * 1000};
  asn_dec_rval_t dec = uper_decode_complete(&ctx, &asn_DEF_LPP_LPP_Message, (void **)&decoded, buffer, n_bytes);

  if (dec.code != RC_OK) {
    fprintf(stderr, "UPER decode failed with code %d\n", dec.code);
    exit(EXIT_FAILURE);
  }

  printf("Decode succeeded. Decoded message:\n");
  xer_fprint(stdout, &asn_DEF_LPP_LPP_Message, decoded);

  ASN_STRUCT_FREE(asn_DEF_LPP_LPP_Message, decoded);
  return 0;
}
