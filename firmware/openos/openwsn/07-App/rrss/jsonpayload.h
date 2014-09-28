#ifndef __JSONPAYLOAD_H
#define __JSONPAYLOAD_H

/**
\addtogroup AppCoAP
\{
\addtogroup rRss
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

//=========================== typedef =========================================

/**
\brief Tracks the state of writing out JSON encoding. Useful for managing syntax. 
*/
typedef enum {
   WRITE_STATE_END_OBJECT    = 0,
   WRITE_STATE_NAME          = 1,
   WRITE_STATE_VALUE         = 2,
   WRITE_STATE_START_OBJECT  = 3,
} write_state_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

void         json_startObject(OpenQueueEntry_t* pkt);
void         json_writeAttributeName(OpenQueueEntry_t* pkt, char* name, uint8_t length);
void         json_writeAttributeInt8Value(OpenQueueEntry_t* pkt, int8_t value);
void         json_writeAttributeUint16Value(OpenQueueEntry_t* pkt, uint16_t value);
void         json_endObject(OpenQueueEntry_t* pkt);


/**
\}
\}
*/

#endif
