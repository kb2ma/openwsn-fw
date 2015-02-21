#include "opendefs.h"
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
#include "jsonpayload.h"
#include "cnethead.h"
#include "rss.h"

//=========================== defines =========================================

// inter-packet period (in mseconds)
#define RSSPERIOD     60000

const uint8_t rss_path0[]  = "nh";
const uint8_t rss_path1[]  = "rss";

//=========================== variables =======================================

typedef struct {
   opentimer_id_t       timerId;
   coap_resource_desc_t desc;
} rss_vars_t;

rss_vars_t rss_vars;

//=========================== prototypes ======================================

owerror_t rss_receive(OpenQueueEntry_t* msg,
                       coap_header_iht*  coap_header,
                       coap_option_iht*  coap_options);
void      rss_task(void);
void      rss_timer(void);
void      rss_sendDone(OpenQueueEntry_t* msg,
                        owerror_t error);
uint8_t   write_int(int val, uint8_t buf[]);
void      reverse(uint8_t* start, uint8_t* end);

//=========================== public ==========================================

void rss_init() {
   // prepare the resource descriptor for the /rss path
   rss_vars.desc.path0len             = sizeof(rss_path0)-1;
   rss_vars.desc.path0val             = (uint8_t*)(&rss_path0);
   rss_vars.desc.path1len             = 0;
   rss_vars.desc.path1val             = NULL;
   rss_vars.desc.componentID          = COMPONENT_CNETHEAD;
   rss_vars.desc.callbackRx           = &rss_receive;
   rss_vars.desc.callbackSendDone     = &rss_sendDone;
   

   rss_vars.timerId    = opentimers_start(RSSPERIOD,
                                          TIMER_PERIODIC,TIME_MS,
                                          rss_timer);
   opencoap_register(&rss_vars.desc);
}

//=========================== private =========================================

owerror_t rss_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   // No supported requests; return an error message
   return E_FAIL;
}

void rss_timer() {
   // Defer task execution from this ISR until CoAP priority.
   scheduler_push_task(rss_task,TASKPRIO_COAP);
}

void rss_task() {    
   OpenQueueEntry_t* pkt;
   owerror_t         outcome;
   neighborRow_t     neighbor;
   uint8_t           i;

   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;
   
   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(rss_vars.timerId);
      return;
   }
   
   // create a CoAP packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_CNETHEAD);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_CNETHEAD,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator    = COMPONENT_CNETHEAD;
   pkt->owner      = COMPONENT_CNETHEAD;
   
   // CoAP payload
   // Write RSS values. We expect at least one value, but allow an empty list.
   json_endObject(pkt);

   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      neighbors_getNeighborInfo(&neighbor, i);
      if (neighbor.used==TRUE) {
         json_writeAttributeInt8Value(pkt, neighbor.rssi);
         json_writeAttributeArray(pkt, &neighbor.addr_64b.addr_64b[6], 2);
      }
   }
   json_startObject(pkt);
   
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = COAP_PAYLOAD_MARKER;
   
   // content-format option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0] = (COAP_OPTION_NUM_CONTENTFORMAT - COAP_OPTION_NUM_URIPATH) << 4 
                     | 1;
   pkt->payload[1] = COAP_MEDTYPE_APPJSON;
   
   // uri-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(rss_path1)-1);
   memcpy(&pkt->payload[0],&rss_path1,sizeof(rss_path1)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = 0 << 4 | (sizeof(rss_path1)-1);
   // uri-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(rss_path0)-1);
   memcpy(&pkt->payload[0],&rss_path0,sizeof(rss_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = COAP_OPTION_NUM_URIPATH << 4 | (sizeof(rss_path0)-1);
   
   // metadata
   pkt->l4_destination_port    = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&nethead_home_addr,16);
   // send
   outcome = opencoap_send(pkt,
                           COAP_TYPE_NON,
                           COAP_CODE_REQ_POST,
                           0,
                           &rss_vars.desc);
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

void rss_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
