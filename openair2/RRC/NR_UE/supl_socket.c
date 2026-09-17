/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "supl_socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/utils/LOG/log.h"
#include "common/utils/threadPool/thread-pool.h"

#include "intertask_interface.h"
#include "mac_messages_types.h"

#define NR_SUPL_SOCKET_PORT 65000
#define NR_SUPL_SOCKET_BACKLOG 5

#define NR_SUPL_HEADER_SIZE 4
#define NR_SUPL_MAX_PAYLOAD_SIZE 60000

static int supl_server_fd = -1;
static instance_t supl_instance_id;


/*
 * Receive exactly len bytes from a TCP socket.
 *
 * TCP is a byte stream, so one recv() call is not guaranteed
 * to return the complete requested amount of data.
 */
static int recv_all(int fd, uint8_t *buffer, size_t len)
{
  size_t received = 0;

  while (received < len) {
    ssize_t ret = recv(fd, buffer + received, len - received, 0);

    if (ret == 0) {
      return 0;
    }

    if (ret < 0) {
      if (errno == EINTR)
        continue;

      LOG_E(NR_RRC,
            "[SUPL] recv() failed: %s\n",
            strerror(errno));
      return -1;
    }

    received += ret;
  }

  return 1;
}


/*
 * Receive and process one framed SUPL payload.
 * Minimal header: bytes 0-3: payload length, network byte order
 */
static int nr_supl_receive_message(int client_fd)
{
  uint8_t header[NR_SUPL_HEADER_SIZE];

  int ret = recv_all(client_fd, header, sizeof(header));

  if (ret <= 0) {
    if (ret == 0)
      LOG_E(NR_RRC, "[SUPL] Connection closed while receiving header\n");

    return -1;
  }

  uint32_t payload_size =
      ((uint32_t)header[0] << 24) |
      ((uint32_t)header[1] << 16) |
      ((uint32_t)header[2] << 8) |
      ((uint32_t)header[3]);

  LOG_I(NR_RRC,
        "[SUPL] Received header: payload=%u bytes\n",
        payload_size);

  if (payload_size == 0 || payload_size > NR_SUPL_MAX_PAYLOAD_SIZE) {
    LOG_E(NR_RRC,
          "[SUPL] Invalid payload size: %u bytes\n",
          payload_size);
    return -1;
  }

  uint8_t *payload = malloc(payload_size);

  if (payload == NULL) {
    LOG_E(NR_RRC,
          "[SUPL] Failed to allocate %u-byte payload buffer\n",
          payload_size);
    return -1;
  }

  ret = recv_all(client_fd, payload, payload_size);

  if (ret <= 0) {
    LOG_E(NR_RRC,
          "[SUPL] Failed to receive %u-byte payload\n",
          payload_size);
    free(payload);
    return -1;
  }

  MessageDef *message_p =
      itti_alloc_new_message(TASK_RRC_NRUE,
                             supl_instance_id,
                             NR_RRC_SUPL_PRS_DATA_IND);

  if (message_p == NULL) {
    LOG_E(NR_RRC,
          "[SUPL] Failed to allocate ITTI message\n");
    free(payload);
    return -1;
  }

  NR_RRC_SUPL_PRS_DATA_IND(message_p).payload_size = payload_size;
  NR_RRC_SUPL_PRS_DATA_IND(message_p).payload = payload;

  if (itti_send_msg_to_task(TASK_RRC_NRUE,
                            supl_instance_id,
                            message_p) < 0) {
    LOG_E(NR_RRC,
          "[SUPL] Failed to send ITTI message\n");

    free(payload);
    free(message_p);
    return -1;
  }

  LOG_I(NR_RRC,
        "[SUPL] Forwarded %u-byte payload to RRC via ITTI\n",
        payload_size);

  return 0;
}


static void *nr_supl_socket_thread(void *arg)
{
  UNUSED(arg);

  LOG_I(NR_RRC,
        "[SUPL] Socket thread started on port %d\n",
        NR_SUPL_SOCKET_PORT);

  if (listen(supl_server_fd, NR_SUPL_SOCKET_BACKLOG) < 0) {
    LOG_E(NR_RRC,
          "[SUPL] listen() failed: %s\n",
          strerror(errno));
    return NULL;
  }

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_fd = accept(supl_server_fd,
                           (struct sockaddr *)&client_addr,
                           &client_addr_len);

    if (client_fd < 0) {
      if (errno == EINTR)
        continue;

      LOG_E(NR_RRC,
            "[SUPL] accept() failed: %s\n",
            strerror(errno));
      continue;
    }

    LOG_I(NR_RRC,
          "[SUPL] Accepted connection from %s:%d\n",
          inet_ntoa(client_addr.sin_addr),
          ntohs(client_addr.sin_port));

    nr_supl_receive_message(client_fd);

    close(client_fd);
  }

  return NULL;
}


void nr_supl_socket_init(instance_t instance_id)
{
  struct sockaddr_in server_addr;
  int optval = 1;

  supl_instance_id = instance_id;

  supl_server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (supl_server_fd < 0) {
    LOG_E(NR_RRC,
          "[SUPL] socket() failed: %s\n",
          strerror(errno));
    return;
  }

  if (setsockopt(supl_server_fd,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 &optval,
                 sizeof(optval)) < 0) {
    LOG_E(NR_RRC,
          "[SUPL] setsockopt() failed: %s\n",
          strerror(errno));
    close(supl_server_fd);
    supl_server_fd = -1;
    return;
  }

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(NR_SUPL_SOCKET_PORT);

  if (bind(supl_server_fd,
           (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    LOG_E(NR_RRC,
          "[SUPL] bind() failed on port %d: %s\n",
          NR_SUPL_SOCKET_PORT,
          strerror(errno));

    close(supl_server_fd);
    supl_server_fd = -1;
    return;
  }

  static pthread_t supl_socket_thread;

  threadCreate(&supl_socket_thread,
               nr_supl_socket_thread,
               NULL,
               "NR/SUPL",
               -1,
               OAI_PRIORITY_RT_LOW);

  LOG_I(NR_RRC,
        "[SUPL] Socket initialized on port %d for UE instance %ld\n",
        NR_SUPL_SOCKET_PORT,
        (long)instance_id);
}
