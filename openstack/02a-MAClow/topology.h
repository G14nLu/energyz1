#ifndef __TOPOLOGY_H
#define __TOPOLOGY_H

/**
\addtogroup MAClow
\{
\addtogroup topology
\{
*/

#include "opendefs.h"
#include "IEEE802154.h"

//=========================== define ==========================================

#define TOPOLOGY_ROOT 	   0x89
#define TOPOLOGY_CHILD_1   0x6e
#define TOPOLOGY_CHILD_2   0xfb
#define TOPOLOGY_CHILD_3   0xae
#define TOPOLOGY_CHILD_4   0x09
#define TOPOLOGY_CHILD_5   0x35
#define TOPOLOGY_CHILD_6   0x8d
#define TOPOLOGY_CHILD_7   0xd2
#define TOPOLOGY_CHILD_8   0xb1
#define TOPOLOGY_CHILD_9   0x67
#define TOPOLOGY_CHILD_10  0x82
#define TOPOLOGY_CHILD_11  0xa0
#define TOPOLOGY_CHILD_12  0x61
#define TOPOLOGY_CHILD_13  0x8a
#define TOPOLOGY_CHILD_14  0xf1

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== prototypes ======================================

bool topology_isAcceptablePacket(ieee802154_header_iht* ieee802514_header);

/**
\}
\}
*/

#endif
