#pragma once

#include <Arduino.h>
#include "IR_patterns.h"
#include "IRLibSendBase.h"    //We need the base code
#include "IRLib_HashRaw.h"    //Only use raw sender

IRsendRaw mySender;

#define WAIT 25

void send_all(uint8_t button_bits){
    switch(button_bits) {

        case 0:
            mySender.send(All_OrangeNop_YellowNop,All_OrangeNop_YellowNop_LEN,76);
            break;

        case 6:
            mySender.send(All_OrangeFwd_YellowFwd,All_OrangeFwd_YellowFwd_LEN,76);
            break;

        case 9:
            mySender.send(All_OrangeRev_YellowRev,All_OrangeRev_YellowRev_LEN,76);
            break;

        case 5:
            mySender.send(All_OrangeFwd_YellowRev,All_OrangeFwd_YellowRev_LEN,76);
            break;

        case 10:
            mySender.send(All_OrangeRev_YellowFwd,All_OrangeRev_YellowFwd_LEN,76);
            break;

        case 2:
            mySender.send(All_OrangeNop_YellowFwd,All_OrangeNop_YellowFwd_LEN,76);
            break;

        case 1:
            mySender.send(All_OrangeNop_YellowRev,All_OrangeNop_YellowRev_LEN,76);
            break;

        case 4:
            mySender.send(All_OrangeFwd_YellowNop,All_OrangeFwd_YellowNop_LEN,76);
            break;

        case 8:
            mySender.send(All_OrangeRev_YellowNop,All_OrangeRev_YellowNop_LEN,76);
            break;

        default:
        Serial.println("INVALID CHAR");
        break;
    }
}