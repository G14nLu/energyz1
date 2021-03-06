#ifndef __SCHEDULE_H
#define __SCHEDULE_H

/**
\addtogroup MAChigh
\{
\addtogroup Schedule
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

/**
\brief The length of the superframe, in slots.

The superframe repeats over time and can be arbitrarly long.
*/
#define SUPERFRAME_LENGTH    11 //should be 101

#define NUMADVSLOTS          1
#define NUMSHAREDTXRX        4
//#define NUMDEDICATEDTX		 14
//#define NUMDEDICATEDRX		 14
#define NUMSERIALRX          1
//
////define slot in trasmissione
//#define SCHEDULE_NODE_START1		6
//#define SCHEDULE_NODE_START2		7
//#define SCHEDULE_NODE_START3		8
//#define SCHEDULE_NODE_START4		9
//#define SCHEDULE_NODE_START5		10
//#define SCHEDULE_NODE_START6		11
//#define SCHEDULE_NODE_START7		12
//#define SCHEDULE_NODE_START8		13
//#define SCHEDULE_NODE_START9		14
//#define SCHEDULE_NODE_START10		15
//#define SCHEDULE_NODE_START11		16
//#define SCHEDULE_NODE_START12		17
//#define SCHEDULE_NODE_START13		18
//#define SCHEDULE_NODE_START14		19
//
////define slot in ricezione
//#define SCHEDULE_NODE_RECEIVE1   20
//#define SCHEDULE_NODE_RECEIVE2   21
//#define SCHEDULE_NODE_RECEIVE3   22
//#define SCHEDULE_NODE_RECEIVE4   23
//#define SCHEDULE_NODE_RECEIVE5   24
//#define SCHEDULE_NODE_RECEIVE6   25
//#define SCHEDULE_NODE_RECEIVE7   26
//#define SCHEDULE_NODE_RECEIVE8   27
//#define SCHEDULE_NODE_RECEIVE9   28
//#define SCHEDULE_NODE_RECEIVE10  29
//#define SCHEDULE_NODE_RECEIVE11  30
//#define SCHEDULE_NODE_RECEIVE12  31
//#define SCHEDULE_NODE_RECEIVE13  32
//#define SCHEDULE_NODE_RECEIVE14  33

/**
\brief Maximum number of active slots in a superframe.

Note that this is merely used to allocate RAM memory for the schedule. The
schedule is represented, in RAM, by a table. There is one row per active slot
in that table; a slot is "active" when it is not of type CELLTYPE_OFF.

Set this number to the exact number of active slots you are planning on having
in your schedule, so not to waste RAM.
*/
#define MAXACTIVESLOTS       (NUMADVSLOTS+NUMSHAREDTXRX+NUMSERIALRX)

/**
\brief Minimum backoff exponent.

Backoff is used only in slots that are marked as shared in the schedule. When
not shared, the mote assumes that schedule is collision-free, and therefore
does not use any backoff mechanism when a transmission fails.
*/
#define MINBE                2

/**
\brief Maximum backoff exponent.

See MINBE for an explanation of backoff.
*/
#define MAXBE                4
//6tisch minimal draft
#define SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS                      5
#define SCHEDULE_MINIMAL_6TISCH_EB_CELLS                          1
#define SCHEDULE_MINIMAL_6TISCH_SLOTFRAME_SIZE                  101
#define SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE          1 //id of slotframe
#define SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_NUMBER          1 //1 slotframe by default.

//=========================== typedef =========================================

typedef uint8_t    channelOffset_t;
typedef uint16_t   slotOffset_t;
typedef uint16_t   frameLength_t;

typedef enum {
   CELLTYPE_OFF              = 255,
   CELLTYPE_ADV              = 0,
   CELLTYPE_TX               = 1,
   CELLTYPE_RX               = 2,
   CELLTYPE_TXRX             = 3,
   CELLTYPE_SERIALRX         = 4,
   CELLTYPE_MORESERIALRX     = 5
} cellType_t;

typedef struct {
   slotOffset_t    slotOffset;
   cellType_t      type;
   bool            shared;
   uint8_t         channelOffset;
   open_addr_t     neighbor;
   uint8_t         numRx;
   uint8_t         numTx;
   uint8_t         numTxACK;
   asn_t           lastUsedAsn;
   void*           next;
} scheduleEntry_t;

BEGIN_PACK
typedef struct {
   uint8_t         row;
   slotOffset_t    slotOffset;
   uint8_t         type;
   bool            shared;
   uint8_t         channelOffset;
   open_addr_t     neighbor;
   uint8_t         numRx;
   uint8_t         numTx;
   uint8_t         numTxACK;
   asn_t           lastUsedAsn;
} debugScheduleEntry_t;
END_PACK

typedef struct {
  uint8_t          address[LENGTH_ADDR64b];
  cellType_t       link_type;
  bool             shared;
  slotOffset_t     slotOffset;
  channelOffset_t  channelOffset;
}slotinfo_element_t;

//=========================== variables =======================================

typedef struct {
   scheduleEntry_t  scheduleBuf[MAXACTIVESLOTS];
   scheduleEntry_t* currentScheduleEntry;
   uint16_t         frameLength;
   uint8_t          backoffExponent;
   uint8_t          backoff;
   uint8_t          debugPrintRow;
} schedule_vars_t;

//=========================== prototypes ======================================

// admin
void               schedule_init(void);
bool               debugPrint_schedule(void);
bool               debugPrint_backoff(void);

// from 6top
void               schedule_setFrameLength(frameLength_t newFrameLength);
owerror_t          schedule_addActiveSlot(
   slotOffset_t         slotOffset,
   cellType_t           type,
   bool                 shared,
   uint8_t              channelOffset,
   open_addr_t*         neighbor
);

void               schedule_getSlotInfo(
   slotOffset_t         slotOffset,                      
   open_addr_t*         neighbor,
   slotinfo_element_t*  info
);

owerror_t          schedule_removeActiveSlot(
   slotOffset_t         slotOffset,
   open_addr_t*         neighbor
);
bool               schedule_isSlotOffsetAvailable(uint16_t slotOffset);

// from IEEE802154E
void               schedule_syncSlotOffset(slotOffset_t targetSlotOffset);
void               schedule_advanceSlot(void);
slotOffset_t       schedule_getNextActiveSlotOffset(void);
frameLength_t      schedule_getFrameLength(void);
cellType_t         schedule_getType(void);
void               schedule_getNeighbor(open_addr_t* addrToWrite);
channelOffset_t    schedule_getChannelOffset(void);
bool               schedule_getOkToSend(void);
void               schedule_resetBackoff(void);
void               schedule_indicateRx(asn_t*   asnTimestamp);
void               schedule_indicateTx(
                        asn_t*    asnTimestamp,
                        bool      succesfullTx
                   );

/**
\}
\}
*/
          
#endif
