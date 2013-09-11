//
// A stub for DOSBox OPL2 emulator.
//


#ifndef BSTONE_DOSBOX_INOUT_H
#define BSTONE_DOSBOX_INOUT_H


#include "dosbox.h"


typedef Bitu IO_ReadHandler(Bitu port, Bitu iolen);
typedef void IO_WriteHandler(Bitu port, Bitu val, Bitu iolen);


class IO_ReadHandleObject {
public:
    void Install(Bitu port, IO_ReadHandler* handler, Bitu mask, Bitu range = 1);
    void Uninstall();
};


class IO_WriteHandleObject {
public:
    void Install(Bitu port, IO_WriteHandler* handler, Bitu mask, Bitu range = 1);
    void Uninstall();
};


#endif // BSTONE_DOSBOX_INOUT_H
