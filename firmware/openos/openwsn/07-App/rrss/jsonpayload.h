#ifndef __JSONPAYLOAD_H
#define __JSONPAYLOAD_H

/**
\addtogroup AppCoAP
\{
\addtogroup rrss
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

void         json_startObject(OpenQueueEntry_t* pkt);
void         json_writeAttributeName(OpenQueueEntry_t* pkt, char* name, uint8_t length);
void         json_writeAttributeInt8Value(OpenQueueEntry_t* pkt, int8_t value);
void         json_endObject(OpenQueueEntry_t* pkt);


/**
\}
\}
*/

#endif
