#pragma once

extern "C" {

#define __CPLUSPLUS

#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <windef.h>
#include <WinBase.h>
#include <winnt.h>

	
	typedef long NTSTATUS;
#define STATUS_SUCCESS 0L

	typedef struct _D3DKMT_CREATEDEVICE {
		VOID *adapter;
		UINT32 flags;
		VOID *device;
		VOID *commandBuffer;
		UINT commandBufferSize;
		VOID* allocationList;
		UINT allocationListSize;
		VOID *patchLocationList;
		UINT pathLocationListSize;

	} D3DKMT_CREATEDEVICE;


	typedef struct _D3DKMT_ESCAPE {
		VOID *adapter;
		VOID *device;
		UINT32 type;
#define D3DKMT_ESCAPE_DRIVERPRIVATE 0

		UINT32 flags;
		VOID *privateDriverData;
		UINT32 privateDriverDataSize;
		VOID *context;
	} D3DKMT_ESCAPE;

	typedef struct _D3DKMT_ADAPTERINFO {
		VOID *handle;
		LUID luid;
		ULONG sourceCount;
		BOOL moveRegionPrefered;
	} D3DKMT_ADAPTERINFO;

	typedef struct _D3DKMT_ENUMADAPTERS {
		ULONG count;
#define MAX_ENUM_ADAPTERS 16
		D3DKMT_ADAPTERINFO adapters[MAX_ENUM_ADAPTERS];
	} D3DKMT_ENUMADAPTERS;

	typedef NTSTATUS(*PFND3DKMT_CREATEDEVICE)(_Inout_ D3DKMT_CREATEDEVICE*);
	typedef NTSTATUS(*PFND3DKMT_ESCAPE)(_In_ const D3DKMT_ESCAPE*);
	typedef NTSTATUS(*PFND3DKMT_ENUMADAPTERS)(_Inout_ const D3DKMT_ENUMADAPTERS*);
}

#include "GLtypes.h"
#include "opengl32.h"