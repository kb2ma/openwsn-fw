#include "openwsn.h"
#include "jsonpayload.h"
#include "packetfunctions.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

uint8_t write_int(int val, uint8_t buf[]);
void reverse(uint8_t* start, uint8_t* end);

//=========================== public ==========================================

void json_startObject(OpenQueueEntry_t* pkt) {
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = '{';
}

void json_endObject(OpenQueueEntry_t* pkt) {
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = '}';
}

void json_writeAttributeName(OpenQueueEntry_t* pkt, char* name, uint8_t length) {
   packetfunctions_reserveHeaderSize(pkt,length+3);
   pkt->payload[0] = '"';
   memcpy(&pkt->payload[1], name, length);
   pkt->payload[length+1] = '"';
   pkt->payload[length+2] = ':';
}

void json_writeAttributeInt8Value(OpenQueueEntry_t* pkt, int8_t value) {
   uint8_t length;
   uint8_t buffer[4];
    
   length = write_int(value, &buffer[0]);
   packetfunctions_reserveHeaderSize(pkt,length);
   memcpy(&pkt->payload[0], &buffer[0], length);
}

//=========================== private =========================================

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
