#pragma once

#include "win_types.h"

/*
    Used to send a preformated command to the VGPU
    No error code. For now we assert on sendCommand if an error occure
*/
void sendCommand(void *command, UINT32 size);