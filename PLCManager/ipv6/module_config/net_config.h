/**
 * @file tcp_ip_stack_config.h
 * @brief CycloneTCP configuration file
 *
 * @section License
 *
 * Copyright (C) 2010-2014 Oryx Embedded. All rights reserved.
 *
 * This file is part of CycloneTCP Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded (www.oryx-embedded.com)
 * @version 1.5.1
 **/

#ifndef _TCP_IP_STACK_CONFIG_H
#define _TCP_IP_STACK_CONFIG_H

#include "os_port_config.h"

#ifdef USE_NO_RTOS
// RTOS-less working mode
#define NET_RTOS_SUPPORT DISABLED
#endif

//Trace level for TCP/IP stack debugging
#define MEM_TRACE_LEVEL          4
#define NIC_TRACE_LEVEL          4
#define ETH_TRACE_LEVEL          2
#define ARP_TRACE_LEVEL          2
#define IP_TRACE_LEVEL           2
#define IPV4_TRACE_LEVEL         2
#define IPV6_TRACE_LEVEL         2
#define ICMP_TRACE_LEVEL         2
#define IGMP_TRACE_LEVEL         4
#define ICMPV6_TRACE_LEVEL       2
#define MLD_TRACE_LEVEL          4
#define NDP_TRACE_LEVEL          4
#define UDP_TRACE_LEVEL          2
#define TCP_TRACE_LEVEL          2
#define SOCKET_TRACE_LEVEL       2
#define RAW_SOCKET_TRACE_LEVEL   2
#define BSD_SOCKET_TRACE_LEVEL   2
#define SLAAC_TRACE_LEVEL        5
#define DHCP_TRACE_LEVEL         4
#define DHCPV6_TRACE_LEVEL       4
#define DNS_TRACE_LEVEL          4
#define MDNS_TRACE_LEVEL         4
#define NBNS_TRACE_LEVEL         4
#define LLMNR_TRACE_LEVEL        4
#define FTP_TRACE_LEVEL          5
#define HTTP_TRACE_LEVEL         4
#define SMTP_TRACE_LEVEL         5
#define SNTP_TRACE_LEVEL         4
#define STD_SERVICES_TRACE_LEVEL 5

//Number of network adapters
#define NET_INTERFACE_COUNT 1

//IPv4 support
#define IPV4_SUPPORT DISABLED

//Ethernet support
#define ETH_SUPPORT DISABLED

//IPv4 fragmentation support
#define IPV4_FRAG_SUPPORT ENABLED
//Maximum number of fragmented packets the host will accept
//and hold in the reassembly queue simultaneously
#define IPV4_MAX_FRAG_DATAGRAMS 4
//Maximum datagram size the host will accept when reassembling fragments
#define IPV4_MAX_FRAG_DATAGRAM_SIZE 8192

//Size of ARP cache
#define ARP_CACHE_SIZE 8
//Maximum number of packets waiting for address resolution to complete
#define ARP_MAX_PENDING_PACKETS 2

//IGMP support
#define IGMP_SUPPORT DISABLED

//IPv6 support
#define IPV6_SUPPORT ENABLED

//IPv6 fragmentation support
#define IPV6_FRAG_SUPPORT ENABLED
//Maximum number of fragmented packets the host will accept
//and hold in the reassembly queue simultaneously
#define IPV6_MAX_FRAG_DATAGRAMS 4
//Maximum datagram size the host will accept when reassembling fragments
#define IPV6_MAX_FRAG_DATAGRAM_SIZE 8192

//MLD support
#define MLD_SUPPORT DISABLED

//Neighbor cache size
#define NDP_CACHE_SIZE 8
//Maximum number of packets waiting for address resolution to complete
#define NDP_MAX_PENDING_PACKETS 2

//G3-specific NDP settings
//Maximum number of Neighbor Solicitation messages sent while performing DAD (0)
#define G3_PLC_DUP_ADDR_DETECT_TRANSMITS 0
//Time interval between retransmissions of RS messages (10s)
#define G3_PLC_RS_RETRY_WAIT_TIME 10000
//Number of retransmissions for RS messages (3)
#define G3_PLC_RS_MAX_RETRY 3

//Some NDP parameters are G3-PLC specific and should be overriden...
#define NDP_DUP_ADDR_DETECT_TRANSMITS G3_PLC_DUP_ADDR_DETECT_TRANSMITS
#define NDP_MIN_RTR_SOLICITATION_DELAY G3_PLC_RS_RETRY_WAIT_TIME
#define NDP_MAX_RTR_SOLICITATION_DELAY G3_PLC_RS_RETRY_WAIT_TIME
#define NDP_RTR_SOLICITATION_INTERVAL G3_PLC_RS_RETRY_WAIT_TIME
#define NDP_MAX_RTR_SOLICITATIONS G3_PLC_RS_MAX_RETRY

//TCP support
#define TCP_SUPPORT DISABLED
//Default buffer size for transmission
#define TCP_DEFAULT_TX_BUFFER_SIZE (1430*2)
//Default buffer size for reception
#define TCP_DEFAULT_RX_BUFFER_SIZE (1430*2)
//Default SYN queue size for listening sockets
#define TCP_DEFAULT_SYN_QUEUE_SIZE 4
//Maximum number of retransmissions
#define TCP_MAX_RETRIES 5
//Selective acknowledgment support
#define TCP_SACK_SUPPORT DISABLED

//UDP support
#define UDP_SUPPORT ENABLED
//Receive queue depth for connectionless sockets
#define UDP_RX_QUEUE_SIZE 4

//Raw socket support
#define RAW_SOCKET_SUPPORT ENABLED
//Receive queue depth for raw sockets
#define RAW_SOCKET_RX_QUEUE_SIZE 4

//Number of sockets that can be opened simultaneously
#define SOCKET_MAX_COUNT 10

// Disable MDNS support
#define MDNS_RESPONDER_SUPPORT DISABLED
#define MDNS_CLIENT_SUPPORT DISABLED

/* Buffer size for PLC application */
#define NET_MEM_POOL_BUFFER_SIZE 128

/* Enable Static Memory Pool for PLC application */
#define NET_MEM_POOL_SUPPORT  ENABLED

#endif
