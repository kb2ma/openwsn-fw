#include "openwsn.h"
#include "jsonpayload.h"
#include "packetfunctions.h"

//=========================== variables =======================================
write_state_t write_state;

//=========================== prototypes ======================================

void writeAttributeUintValue(OpenQueueEntry_t* pkt, unsigned int value, uint8_t sign);
uint8_t write_uint(unsigned int val, uint8_t sign, uint8_t buf[]);
void reverse(uint8_t* start, uint8_t* end);

//=========================== public ==========================================

/**
\brief Writes the opening brace.

\param[in] pkt Writes to the payload member of this packet
 */
void json_startObject(OpenQueueEntry_t* pkt) {
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = '{';
   write_state = WRITE_STATE_START_OBJECT;
}

/**
\brief Writes the closing brace.

\param[in] pkt Writes to the payload member of this packet
 */
void json_endObject(OpenQueueEntry_t* pkt) {
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = '}';
   write_state = WRITE_STATE_END_OBJECT;
}

/**
\brief Writes the provided name and colon for an object attribute.

\param[in] pkt Writes to the payload member of this packet
\param[in] name Name to write
\param[in] length Length of name
 */
void json_writeAttributeName(OpenQueueEntry_t* pkt, char* name, uint8_t length) {
   packetfunctions_reserveHeaderSize(pkt,length+3);
   pkt->payload[0] = '"';
   memcpy(&pkt->payload[1], name, length);
   pkt->payload[length+1] = '"';
   pkt->payload[length+2] = ':';
   write_state = WRITE_STATE_NAME;
}

/**
\brief Writes the provided \c uint16_t value.

\param[in] pkt Writes to the payload member of this packet
\param[in] value Value to write
 */
void json_writeAttributeUint16Value(OpenQueueEntry_t* pkt, uint16_t value) {
   writeAttributeUintValue(pkt, (unsigned int)value, 0);
}

/**
\brief Writes the provided \c int8_t value.

\param[in] pkt Writes to the payload member of this packet
\param[in] value Value to write
 */
void json_writeAttributeInt8Value(OpenQueueEntry_t* pkt, int8_t value) {
   writeAttributeUintValue(pkt, (unsigned int)(value<0 ? -value : value), value<0);
}

//=========================== private =========================================

/**
\brief Appends a comma if followed by another attribute.
 
\param[in] pkt Writes to the payload member of this packet
\param[in] value Value to write
\param[in] sign TRUE if < 0, so prints a negative sign
 */
void writeAttributeUintValue(OpenQueueEntry_t* pkt, unsigned int value, uint8_t sign) {
   uint8_t length;
   uint8_t buffer[5];

   if (write_state == WRITE_STATE_NAME) {
      packetfunctions_reserveHeaderSize(pkt,1);
      pkt->payload[0] = ',';
   }
    
   length = write_uint(value, sign, &buffer[0]);
   packetfunctions_reserveHeaderSize(pkt,length);
   memcpy(&pkt->payload[0], &buffer[0], length);
   write_state = WRITE_STATE_VALUE;
}

/** 
\brief Writes the provided value as ASCII chars to the provided buffer. 

\param[in] value Value to write
\param[in] sign TRUE if < 0, so prints a negative sign
\param[in,out] buf Array target for output
\return Count of characters written
 */
uint8_t write_uint(unsigned int value, uint8_t sign, uint8_t buf[]) {
   uint8_t i;

   // Generates digits little end first, and then reverses
   i = 0;
   do {
      buf[i++] = value % 10 + '0';
      value   /= 10;
   } while (value > 0);
   
   if (sign) {
      buf[i++] = '-';
   }
   reverse(&buf[0], &buf[i-1]);
   
   return i;
}

/**
\brief Reverses buffer in place  
*/
void reverse(uint8_t* start, uint8_t* end) {
   uint8_t c;

   while(start < end) {
      c        = *start;
      *start++ = *end;
      *end--   = c;
   }
}
