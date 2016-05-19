#include "opendefs.h"
#include "udplatency.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "opentimers.h"
#include "openrandom.h"
#include "opencoap.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "idmanager.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"

//=========================== defines =========================================

//#define UDPLATENCYPERIOD 30
//#define NUMPKTTEST 120

//=========================== variables =======================================

udplatency_vars_t udplatency_vars;
//=========================== prototypes ======================================

void udplatency_timer(void);
void udplatency_pushTimer(void);
void udplatency_PushTask(void);
void trigger_forward(void);

//=========================== public ==========================================

void udplatency_init(void) {
   udplatency_vars.seqNum_my     		= 0;
   udplatency_vars.seqNum_global 		= 0;
   udplatency_vars.udplatency_security 	= 0;
   udplatency_vars.triggerReceived 		= 0;

   udplatency_vars.UDPLATENCYPERIOD = 2000;
   udplatency_vars.NUMPKTTEST = 300;

   udplatency_vars.timerId    = opentimers_start(udplatency_vars.UDPLATENCYPERIOD,
   												TIMER_PERIODIC,TIME_MS,
   												udplatency_timer);
}

void udplatency_task() {
   OpenQueueEntry_t* pkt;
   open_addr_t * p;
   open_addr_t  q;

   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;

   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
       opentimers_stop(udplatency_vars.timerId);
       return;
   }

   // prepare packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_UDPLATENCY);
   if (pkt==NULL) {
//	   openserial_printError(COMPONENT_UDPLATENCY,ERR_NO_FREE_PACKET_BUFFER,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
	   // increment seqNum, PLR stats on OV
	   udplatency_vars.seqNum_my++;
	   udplatency_vars.seqNum_global++;
      return;
   }

   pkt->creator                     = COMPONENT_UDPLATENCY;
   pkt->owner                       = COMPONENT_UDPLATENCY;
   pkt->l4_protocol                 = IANA_UDP;
   pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_LATENCY;
   pkt->l4_destination_port         = WKP_UDP_LATENCY;
   pkt->l3_destinationAdd.type      = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motedata,16);

   // the payload contains the 64bit address of the sender + the ASN
   packetfunctions_reserveHeaderSize(pkt, sizeof(asn_t));
   ieee154e_getAsn(pkt->payload);//gets asn from mac layer.
   
   packetfunctions_reserveHeaderSize(pkt,8);
   p=idmanager_getMyID(ADDR_64B);
   pkt->payload[0]    = p->addr_64b[0];
   pkt->payload[1]    = p->addr_64b[1];
   pkt->payload[2]    = p->addr_64b[2];
   pkt->payload[3]    = p->addr_64b[3];
   pkt->payload[4]    = p->addr_64b[4];
   pkt->payload[5]    = p->addr_64b[5];
   pkt->payload[6]    = p->addr_64b[6];
   pkt->payload[7]    = p->addr_64b[7];
   

   neighbors_getPreferredParentEui64(&q);
   if (q.type==ADDR_64B) {
      packetfunctions_reserveHeaderSize(pkt,8);
   
   // copy my preferred parent so we can build the topology
      pkt->payload[0] = q.addr_64b[0];
      pkt->payload[1] = q.addr_64b[1];
      pkt->payload[2] = q.addr_64b[2];
      pkt->payload[3] = q.addr_64b[3];
      pkt->payload[4] = q.addr_64b[4];
      pkt->payload[5] = q.addr_64b[5];
      pkt->payload[6] = q.addr_64b[6];
      pkt->payload[7] = q.addr_64b[7];
   }

   // insert Sequence Number
   packetfunctions_reserveHeaderSize(pkt,sizeof(udplatency_vars.seqNum_global));
   pkt->payload[0]    = (udplatency_vars.seqNum_global >> 8) & 0xff;
   pkt->payload[1]    = udplatency_vars.seqNum_global & 0xff;

   pkt->FIFO_seqNum = udplatency_vars.seqNum_my;

   openserial_printInfo(COMPONENT_UDPLATENCY,155,
					   (errorparameter_t)pkt->FIFO_seqNum,
					   (errorparameter_t)100);
   //127 bytes payload
   uint8_t i;
   for (i = 0; i < 16; i++){
	   packetfunctions_reserveHeaderSize(pkt,1);
	   pkt->payload[0] = i;
   }


   // send packet
   if ((openudp_send(pkt)) == E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }

   // increment seqNum
   udplatency_vars.seqNum_my++;
   udplatency_vars.seqNum_global++;

   // close timer when test finish
   if (udplatency_vars.seqNum_my > udplatency_vars.NUMPKTTEST) {
	   udplatency_vars.triggerReceived = FALSE;
	   udplatency_vars.seqNum_my = 0;
	   udplatency_vars.seqNum_global = 0;
       opentimers_stop(udplatency_vars.timerId);
   }

}

void udplatency_timer(void) {
  scheduler_push_task(udplatency_task,TASKPRIO_COAP);
}

void udplatency_pushTimer(void){
  scheduler_push_task(udplatency_PushTask,TASKPRIO_SIXTOP);
}

void udplatency_forwardTimer(void){
	scheduler_push_task(trigger_forward,TASKPRIO_SIXTOP);
}

void udplatency_PushTask(void){
	udplatency_vars.timerId    = opentimers_start(udplatency_vars.UDPLATENCYPERIOD,
												TIMER_PERIODIC,TIME_MS,
												udplatency_timer);
}

void udplatency_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_UDPLATENCY;
   if (msg->creator!=COMPONENT_UDPLATENCY) {
      /*openserial_printError(COMPONENT_UDPLATENCY,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);*/
   }
   openqueue_freePacketBuffer(msg);
}

void udplatency_receive(OpenQueueEntry_t* msg) {
   openqueue_freePacketBuffer(msg);
}

void trigger_receive(OpenQueueEntry_t* msg){

	//TODO
	uint16_t receivedRate;
	uint16_t numberOfPackets;
	uint8_t  securityFlag;
	uint16_t timeToWaitReceived;

	if(idmanager_getIsDAGroot()){
		udplatency_vars.triggerReceived = TRUE;
	}

	//if I have received the desync message previously, simply discard it.
	if (udplatency_vars.triggerReceived == TRUE){
		//free the RAM
		openqueue_freePacketBuffer(msg);

		return;
	} else {
		udplatency_vars.triggerReceived = TRUE;
	}

    openserial_printError(COMPONENT_UDPLATENCY,ERR_INVALIDSERIALFRAME,
                  (errorparameter_t)0,
                  (errorparameter_t)500);

    //toss the protocol header
	packetfunctions_tossHeader(msg,1);

	//retrieve values
	//get the rate
	receivedRate = msg->payload[0] + 256 * msg->payload[1];
	packetfunctions_tossHeader(msg,2);

	//get the number of packets to generate
	numberOfPackets = msg->payload[0] + 256 * msg->payload[1];
	packetfunctions_tossHeader(msg,2);

	//get the security flag
	securityFlag = msg->payload[0];
	packetfunctions_tossHeader(msg,1);

	//get the time to wait
	timeToWaitReceived = msg->payload[0] + 256 * msg->payload[1];
	packetfunctions_tossHeader(msg,2);

	//free up the RAM
	openqueue_freePacketBuffer(msg);

	//save variables
	udplatency_vars.UDPLATENCYPERIOD    = receivedRate;
	udplatency_vars.NUMPKTTEST          = numberOfPackets;
	udplatency_vars.udplatency_security = securityFlag;
	udplatency_vars.timeToWaitReceived  = timeToWaitReceived;

	//schedule the timer for the start of the UDPLatency task
	udplatency_vars.globaltimerId = opentimers_start(timeToWaitReceived,
													 TIMER_ONESHOT,TIME_MS,
													 udplatency_pushTimer);

	//forward the packet
//	udplatency_vars.timerTriggerForward = opentimers_start(50,
//			 	 	 	 	 	 	 	 	 	 	 	TIMER_ONESHOT,TIME_MS,
//														udplatency_forwardTimer);
//	return;
}


uint16_t udplatency_getSeqNum(void){
	uint16_t value;
	value = udplatency_vars.seqNum_global;
	udplatency_vars.seqNum_global++;
	return value;
}

uint8_t udplatency_getTimerId(void){
	return udplatency_vars.timerId;
}

void udplatency_setSecurity(bool value){
	udplatency_vars.udplatency_security = value;
}

bool udplatency_getSecurity(void){
	return udplatency_vars.udplatency_security;
}

void udplatency_setPeriod(uint16_t value){
	udplatency_vars.UDPLATENCYPERIOD = value;
}

/*
 * Forward the trigger message down in the tree
 */
void trigger_forward(void){

	OpenQueueEntry_t* pkt;

    //generate a broadcast MAC message with received parameters
    pkt = openqueue_getFreePacketBuffer(COMPONENT_OPENBRIDGE);
    if (pkt==NULL) {
//	  openserial_printError(COMPONENT_UDPLATENCY,ERR_NO_FREE_PACKET_BUFFER,
//						   (errorparameter_t)0,
//						   (errorparameter_t)2);
	  return;
    }

    openserial_printError(COMPONENT_UDPLATENCY,ERR_INVALIDSERIALFRAME,
                  (errorparameter_t)0,
                  (errorparameter_t)501);

   //admin
   pkt->creator  = COMPONENT_SIXTOP;
   pkt->owner    = COMPONENT_UDPLATENCY;

   // some l2 information about this packet
   pkt->l2_frameType                     = IEEE154_TYPE_DATA;
   pkt->l2_nextORpreviousHop.type        = ADDR_16B;
   pkt->l2_nextORpreviousHop.addr_16b[0] = 0xff;
   pkt->l2_nextORpreviousHop.addr_16b[1] = 0xff;
   pkt->isBroadcastIE                    = TRUE;
   //   pkt->FIFO_sn 						 = 0; //maximum priority

   //payload
   //amount of time we have to wait for the start of sending packets
   packetfunctions_reserveHeaderSize(pkt, sizeof(uint16_t));
   pkt->payload[0] = (uint8_t) udplatency_vars.timeToWaitReceived;
   pkt->payload[1] = (uint8_t) (udplatency_vars.timeToWaitReceived >> 8);

   //security flag
   packetfunctions_reserveHeaderSize(pkt, sizeof(uint8_t));
   pkt->payload[0] = udplatency_vars.udplatency_security; //list-termination

   //number of packets
   packetfunctions_reserveHeaderSize(pkt, sizeof(uint16_t));
   pkt->payload[0] = (uint8_t) udplatency_vars.NUMPKTTEST;
   pkt->payload[1] = (uint8_t) (udplatency_vars.NUMPKTTEST >> 8);

   //rate
   packetfunctions_reserveHeaderSize(pkt, sizeof(uint16_t));
   pkt->payload[0] = (uint8_t) udplatency_vars.UDPLATENCYPERIOD;
   pkt->payload[1] = (uint8_t) (udplatency_vars.UDPLATENCYPERIOD >> 8);

   //add id for the protocol
   packetfunctions_reserveHeaderSize(pkt, sizeof(uint8_t));
   pkt->payload[0] = 0xAB;

   // put in queue for MAC to handle
   sixtop_send(pkt);

   return;

}

//=========================== private =========================================
