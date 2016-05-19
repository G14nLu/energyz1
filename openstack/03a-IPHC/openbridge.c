#include "opendefs.h"
#include "openbridge.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "iphc.h"
#include "idmanager.h"
#include "openqueue.h"
#include "sixtop.h"

//=========================== variables =======================================

//=========================== define ==========================================
//id of the trigger protocol
#define TRIGGER_PROTOCOL 50
//wait time for the start in msec
#define WAIT_TIME 5000
//=========================== prototypes ======================================
//=========================== public ==========================================

void openbridge_init() {
}

void openbridge_triggerData() {
   uint8_t           input_buffer[136];//worst case: 8B of next hop + 128B of data
   OpenQueueEntry_t* pkt;
   uint8_t           numDataBytes;
  
   numDataBytes = openserial_getNumDataBytes();
  
   //poipoi xv
   //this is a temporal workaround as we are never supposed to get chunks of data
   //longer than input buffer size.. I assume that HDLC will solve that.
   // MAC header is 13B + 8 next hop so we cannot accept packets that are longer than 118B
   if (numDataBytes>(136 - 21) || numDataBytes<8){
   //to prevent too short or too long serial frames to kill the stack  
//       openserial_printError(COMPONENT_OPENBRIDGE,ERR_INPUTBUFFER_LENGTH,
//                   (errorparameter_t)numDataBytes,
//                   (errorparameter_t)1);
       return;
   }
  
   //copying the buffer once we know it is not too big
   openserial_getInputBuffer(&(input_buffer[0]),numDataBytes);
  
   if (idmanager_getIsDAGroot()==TRUE && numDataBytes>0) {
	   if(input_buffer[1] != TRIGGER_PROTOCOL){
		  pkt = openqueue_getFreePacketBuffer(COMPONENT_OPENBRIDGE);
		  if (pkt==NULL) {
			 openserial_printError(COMPONENT_OPENBRIDGE,ERR_NO_FREE_PACKET_BUFFER,
								   (errorparameter_t)0,
								   (errorparameter_t)2);
			 return;
		  }
		  //admin
		  pkt->creator  = COMPONENT_OPENBRIDGE;
		  pkt->owner    = COMPONENT_OPENBRIDGE;
		  //l2
		  pkt->l2_nextORpreviousHop.type = ADDR_64B;
		  memcpy(&(pkt->l2_nextORpreviousHop.addr_64b[0]),&(input_buffer[0]),8);
		  //payload
		  packetfunctions_reserveHeaderSize(pkt,numDataBytes-8);
		  memcpy(pkt->payload,&(input_buffer[8]),numDataBytes-8);

		  //this is to catch the too short packet. remove it after fw-103 is solved.
		  if (numDataBytes<16){
				  openserial_printError(COMPONENT_OPENBRIDGE,ERR_INVALIDSERIALFRAME,
								(errorparameter_t)0,
								(errorparameter_t)3);
		  }
		  //send
		  if ((iphc_sendFromBridge(pkt))==E_FAIL) {
			 openqueue_freePacketBuffer(pkt);
		  }
	   } else { //this is my trigger protocol
		   //TODO
	       uint16_t no_packets;
	       uint16_t period;
	       uint8_t  securityEnabled;
	       //identify the rate
	       switch(input_buffer[2]){
	       case 0:
	    	   period = 5000;
	    	   break;
	       case 1:
	    	   period = 1000;
	    	   break;
	       case 2:
	    	   period = 100;
	       default:
	    	   break;
	       }

	       //identify the number of packets
	       switch(input_buffer[3]){
	       case 0:
	    	   no_packets = 256;
	    	   break;
	       case 1:
	    	   no_packets = 500;
	    	   break;
	       case 2:
	    	   no_packets = 750;
	    	   break;
	       default:
	    	   no_packets = 1000;
	    	   break;
	       }
	       //no_packets = input_buffer[3];

	       //identify security flag
	       securityEnabled = input_buffer[4];

	       openserial_printError(COMPONENT_OPENBRIDGE,ERR_INVALIDSERIALFRAME,
	                     (errorparameter_t)period,
	                     (errorparameter_t)no_packets);
	       openserial_printError(COMPONENT_OPENBRIDGE,ERR_INVALIDSERIALFRAME,
	                     (errorparameter_t)securityEnabled,
	                     (errorparameter_t)0);
	       //generate a broadcast MAC message with received parameters
		   pkt = openqueue_getFreePacketBuffer(COMPONENT_OPENBRIDGE);
		   if (pkt==NULL) {
			  openserial_printError(COMPONENT_OPENBRIDGE,ERR_NO_FREE_PACKET_BUFFER,
								   (errorparameter_t)0,
								   (errorparameter_t)2);
			  return;
		   }
		   //admin
		   pkt->creator  = COMPONENT_OPENBRIDGE;
		   pkt->owner    = COMPONENT_OPENBRIDGE;

		   // some l2 information about this packet
		   pkt->l2_frameType                     = IEEE154_TYPE_DATA;
		   pkt->l2_nextORpreviousHop.type        = ADDR_16B;
		   pkt->l2_nextORpreviousHop.addr_16b[0] = 0xff;
		   pkt->l2_nextORpreviousHop.addr_16b[1] = 0xff;
		   pkt->isBroadcastIE                    = TRUE;

		   //payload
		   //amount of time we have to wait for the start of sending packets
		   packetfunctions_reserveHeaderSize(pkt, sizeof(uint16_t));
		   pkt->payload[0] = (uint8_t) WAIT_TIME;
		   pkt->payload[1] = (uint8_t) (WAIT_TIME >> 8);

		   //security flag
		   packetfunctions_reserveHeaderSize(pkt, sizeof(uint8_t));
		   pkt->payload[0] = securityEnabled; //list-termination

		   //number of packets
		   packetfunctions_reserveHeaderSize(pkt, sizeof(uint16_t));
		   pkt->payload[0] = (uint8_t) no_packets;
		   pkt->payload[1] = (uint8_t) (no_packets >> 8);

		   //rate
		   packetfunctions_reserveHeaderSize(pkt, sizeof(uint16_t));
		   pkt->payload[0] = (uint8_t) period;
		   pkt->payload[1] = (uint8_t) (period >> 8);

		   //add id for the protocol
		   packetfunctions_reserveHeaderSize(pkt, sizeof(uint8_t));
		   pkt->payload[0] = 0xAB;

		   // put in queue for MAC to handle
		   sixtop_send(pkt);

	       return;
	   }
   } else {
       openserial_printError(COMPONENT_OPENBRIDGE,ERR_INVALIDSERIALFRAME,
                     (errorparameter_t)255,
                     (errorparameter_t)255);
   }
}

void openbridge_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_OPENBRIDGE;
   if (msg->creator!=COMPONENT_OPENBRIDGE) {
      openserial_printError(COMPONENT_OPENBRIDGE,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

/**
\brief Receive a frame at the openbridge, which sends it out over serial.
*/
void openbridge_receive(OpenQueueEntry_t* msg) {
   
   // prepend previous hop
   packetfunctions_reserveHeaderSize(msg,LENGTH_ADDR64b);
   memcpy(msg->payload,msg->l2_nextORpreviousHop.addr_64b,LENGTH_ADDR64b);
   
   // prepend next hop (me)
   packetfunctions_reserveHeaderSize(msg,LENGTH_ADDR64b);
   memcpy(msg->payload,idmanager_getMyID(ADDR_64B)->addr_64b,LENGTH_ADDR64b);
   
   // send packet over serial (will be memcopied into serial buffer)
   openserial_printData((uint8_t*)(msg->payload),msg->length);
   
   // free packet
   openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================
