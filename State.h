#ifndef STATE_H
#define STATE_H

enum DeviceState
{
    STATE_BOOTUP,
    STATE_POWERSAFE,
    STATE_POWERSAFE_COMMUNICATING,
    STATE_NORMAL,
    STATE_OVERPOWER,
};

extern DeviceState deviceState;

#endif