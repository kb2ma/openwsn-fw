/*
 * Copyright (c) 2014, Ken Bannister
 * All rights reserved. 
 *  
 * Released under the Mozilla Public License 2.0, as published at the link below.
 * http://opensource.org/licenses/MPL-2.0
 */
#include "cnethead.h"
#include "hello.h"
#include "rss.h"

//=========================== defines =========================================

// Address of the nethead client for phone home
const uint8_t nethead_home_addr[] = {0xfd, 0xc8, 0x70, 0xa6, 0x51, 0x1c, 0x00, 0x00, \
                                     0x22, 0x1a, 0x06, 0xff, 0xfe, 0x03, 0xca, 0xf6};

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

/**
\brief Initializes the Nethead application
 */
void cnethead_init() {
   hello_init();
}

/**
\brief Notifies that this mote has been registered with Nethead home.

Sent by the hello module. This notification could be reimplemented as a defined
callback to avoid exposing this function in cnethead's interface. However, this
notification is internal to the relatively small nethead application, so not a
pressing concern.
 */
void cnethead_notify_registered() {
   rss_init();
}
//=========================== private =========================================
