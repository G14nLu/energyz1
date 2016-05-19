#ifndef __UDPLATENCY_H
#define __UDPLATENCY_H

#include "opendefs.h"
#include "opentimers.h"

/**
\addtogroup App

\addtogroup udpLatency
\{
*/

//=========================== define ==========================================
//=========================== typedef =========================================
typedef struct {
	opentimer_id_t  globaltimerId;        //id of the global timer for the udplatency task
	opentimer_id_t  timerId;              //id of the timer for the udplatency task
	opentimer_id_t  timerTriggerForward;  //id of the timer for the forward of the trigger message
	uint16_t 		UDPLATENCYPERIOD;     //period sending packets
	uint16_t        NUMPKTTEST;			  //number of packets to be sent by this node
	uint16_t        seqNum_my;            //local counter, all packets originated by this node
	uint16_t        seqNum_global;        //global counter, all packets sent by this node
	bool			udplatency_security;  //flag for security enabled/disabled
	uint16_t        timeToWaitReceived;   //time to wait for the start of the udplatency task
	bool            triggerReceived;      //flag that signals if the trigger signal have been received
} udplatency_vars_t;
//=========================== variables =======================================

//=========================== prototypes ======================================

void udplatency_init(void);
void udplatency_trigger(void);
void udplatency_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void udplatency_receive(OpenQueueEntry_t* msg);
bool udplatency_debugPrint(void);
void udplatency_task(void);

void trigger_receive(OpenQueueEntry_t* msg);

uint16_t udplatency_getSeqNum(void);
uint8_t udplatency_getTimerId(void);
void udplatency_setSecurity(bool value);
void udplatency_setPeriod(uint16_t value);
bool udplatency_getSecurity(void);

/**
\}
\}
*/

#endif
