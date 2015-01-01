/*
 * Copyright (c) 2014, Ken Bannister
 * All rights reserved. 
 *  
 * Released under the Mozilla Public License 2.0, as published at the link below.
 * http://opensource.org/licenses/MPL-2.0
 */
#ifndef __JSONPAYLOAD_H
#define __JSONPAYLOAD_H

/**
\addtogroup AppCoAP
\{
\addtogroup nethead
\{
\defgroup json json
\brief Functions to write JSON encoded data to a payload.

Writes directly to a packet, and so is designed to add sytactic elements in reverse
order. Internally maintains the state of JSON syntax.
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

void         json_startObject(OpenQueueEntry_t* pkt);
void         json_writeAttributeName(OpenQueueEntry_t* pkt, char* name, uint8_t length);
void         json_writeAttributeInt8Value(OpenQueueEntry_t* pkt, int8_t value);
void         json_writeAttributeUint16Value(OpenQueueEntry_t* pkt, uint16_t value);
void         json_writeAttributeArray(OpenQueueEntry_t* pkt, uint8_t* array, uint8_t length);
void         json_endObject(OpenQueueEntry_t* pkt);


/**
\}
\}
\}
*/

#endif
