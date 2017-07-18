#pragma once

#include "win_types.h"

/*
    Used to send a preformated command to the VGPU
    No error code. For now we assert on sendCommand if an error occure
*/
VOID sendCommand(VOID *command, UINT32 size);

UINT64 allocate_object(UINT32 size);
VOID update_object(UINT64 handle, VOID *data, UINT32 size);
VOID delete_object(UINT64 handle);