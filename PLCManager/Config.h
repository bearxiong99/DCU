#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "conf_bs.h"

/* Port number for the application */
#define APP_SOCKET_PORT 0xF0B1

/* Generic G3 IPv6 coordinator address */
#define APP_IPV6_GENERIC_COORDINATOR_ADDR "fe80::7455:ff:fe00:0000"

/* Generic G3 IPv6 local-link address */
#define APP_IPV6_GENERIC_LINK_LOCAL_ADDR "fe80::7455:ff:fe00:0001"

/* Default timeout for the application */
#define APP_DEFAULT_TIMEOUT 5000

// Coordinator short address
#define CONF_SHORT_ADDRESS (const uint8_t *) "\x00\x00"

// PSK / Network authentication Key (16 bytes)
#define CONF_PSK_KEY (const uint8_t *) "\xAB\x10\x34\x11\x45\x11\x1B\xC3\xC1\x2D\xE8\xFF\x11\x14\x22\x04"

// GMK (16 bytes)
#define CONF_GMK_KEY (const uint8_t *) "\xAF\x4D\x6D\xCC\xF1\x4D\xE7\xC1\xC4\x23\x5E\x6F\xEF\x6C\x15\x1F"

// Context information table: index 0 (Context 0 with value c_IPv6_PREFIX & x_PAN_ID (length = 80))
#define CONF_CONTEXT_INFORMATION_TABLE_0 (const uint8_t *) "\x02\x00" "\x01" "\x50" "\xFE\x80\x00\x00\x00\x00\x00\x00\x78\x1D"

// Context information table: index 1 (Context 1 with value �112233445566� (length = 48))
#define CONF_CONTEXT_INFORMATION_TABLE_1 (const uint8_t *) "\x02\x00" "\x01" "\x30" "\x11\x22\x33\x44\x55\x66"

// Routing table entry TTL (2 bytes, little endian) (180 minutes)
#define CONF_ROUTING_TABLE_ENTRY_TTL (const uint8_t *) "\xB4\x00"

// GroupTable: index 0 (2 bytes, little endian) (0x8567 � note that the IPv6 layer must listen to ff12:30:1122:3344:5566:0:123:4567 in correspondence to this group)
#define CONF_GROUP_TABLE_0 (const uint8_t *) "\x67\x85"

// Max Join Time: 90 seconds
#define CONF_MAX_JOIN_WAIT_TIME (const uint8_t *) "\x5A"

// Max Hops: 10
#define CONF_MAX_HOPS (const uint8_t *) "\x0A"

// DEVICE MAX. NUMBER
#define G3_MAX_DEV             MAX_LBDS

#endif
