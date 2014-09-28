#ifndef __RRSS_H
#define __RRSS_H

/**
\addtogroup AppCoAP
\{
\addtogroup rRss
\{
*/

#include "opencoap.h"

//=========================== define ==========================================
/**
\brief CoAP monitoring server
 */
static const uint8_t ipAddr_monServer[] = {0xfd, 0xc8, 0x70, 0xa6, 0x51, 0x1c, 0x00, 0x00, \
                                           0x22, 0x1a, 0x06, 0xff, 0xfe, 0x03, 0xca, 0xf6};

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void rrss_init(void);

/**
\}
\}
*/

#endif
