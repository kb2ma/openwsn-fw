#include "openwsn.h"
#include "rrss.h"
#include "opentimers.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "neighbors.h"
#include "idmanager.h"
#include "IEEE802154E.h"
#include "scheduler.h"


//=========================== defines =========================================

/// inter-packet period (in mseconds)
#define RRSSPERIOD     60000

const uint8_t rrss_path0[] = "rss";

//=========================== variables =======================================

typedef struct {
   opentimer_id_t       timerId;
   coap_resource_desc_t desc;
} rrss_vars_t;

rrss_vars_t rrss_vars;

//=========================== prototypes ======================================

owerror_t rrss_receive(OpenQueueEntry_t* msg,
                       coap_header_iht*  coap_header,
                       coap_option_iht*  coap_options);
void      rrss_task(void);
void      rrss_timer(void);
void      rrss_sendDone(OpenQueueEntry_t* msg,
                        owerror_t error);
uint8_t   write_int(int val, uint8_t buf[]);
void      reverse(uint8_t* start, uint8_t* end);

//=========================== public ==========================================

void rrss_init() {
   // prepare the resource descriptor for the /rss path
   rrss_vars.desc.path0len             = sizeof(rrss_path0)-1;
   rrss_vars.desc.path0val             = (uint8_t*)(&rrss_path0);
   rrss_vars.desc.path1len             = 0;
   rrss_vars.desc.path1val             = NULL;
   rrss_vars.desc.componentID          = COMPONENT_RRSS;
   rrss_vars.desc.callbackRx           = &rrss_receive;
   rrss_vars.desc.callbackSendDone     = &rrss_sendDone;
   

   rrss_vars.timerId    = opentimers_start(RRSSPERIOD,
                                          TIMER_PERIODIC,TIME_MS,
                                          rrss_timer);
   opencoap_register(&rrss_vars.desc);
}

//=========================== private =========================================

owerror_t rrss_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   // No supported requests; return an error message
   return E_FAIL;
}

void rrss_timer() {
   // Defer task execution from this ISR until CoAP priority.
   scheduler_push_task(rrss_task,TASKPRIO_COAP);
}

void rrss_task() {    
   OpenQueueEntry_t* pkt;
   owerror_t         outcome;
   uint8_t           numOptions, i;
   netDebugNeigborEntry_t neighbor;
   uint8_t           rssbuf[4];

   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;
   
   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(rrss_vars.timerId);
      return;
   }
   
   // create a CoAP packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_RRSS);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_RRSS,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator    = COMPONENT_RRSS;
   pkt->owner      = COMPONENT_RRSS;
   // CoAP payload
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = COAP_PAYLOAD_MARKER;
   
   memset(&neighbor, 0, sizeof(netDebugNeigborEntry_t));
   debugNetPrint_neighbors(&neighbor);
   
   packetfunctions_reserveHeaderSize(pkt,5);
   pkt->payload[0] = '{';
   pkt->payload[1] = '"';
   pkt->payload[2] = 'v';
   pkt->payload[3] = '"';
   pkt->payload[4] = ':';
   
   i = write_int(neighbor.rssi, rssbuf);
   packetfunctions_reserveHeaderSize(pkt,i+1);
   memcpy(&pkt->payload[0],&rssbuf[0],i);
   
   pkt->payload[i] = '}';
   
   numOptions = 0;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(rrss_path0)-1);
   memcpy(&pkt->payload[0],&rrss_path0,sizeof(rrss_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
      (sizeof(rrss_path0)-1);
   numOptions++;
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 | 1;
   pkt->payload[1]                  = COAP_MEDTYPE_APPJSON;
   numOptions++;
   // metadata
   pkt->l4_destination_port         = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motesEecs,16);
   // send
   outcome = opencoap_send(pkt,
                           COAP_TYPE_NON,
                           COAP_CODE_REQ_POST,
                           numOptions,
                           &rrss_vars.desc);
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

void rrss_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

/** 
 * Writes the provided value as ASCII chars to the provided buffer. Returns
 * the count of characters written.
 */
uint8_t write_int(int val, uint8_t buf[]) {
   int sign;
   uint8_t i;

   sign = val;
   if (sign < 0) {
      val = -val;
   }

   // Generates digits little end first, and then reverses
   i = 0;
   do {
      buf[i++] = val % 10 + '0';
      val   /= 10;
   } while (val > 0);
   
   if (sign < 0) {
      buf[i++] = '-';
   }
   reverse(&buf[0], &buf[i-1]);
   
   return i;
}

/** Reverses buffer in place  */
void reverse(uint8_t* start, uint8_t* end) {
   uint8_t c;

   while(start < end) {
      c        = *start;
      *start++ = *end;
      *end--   = c;
   }
}
