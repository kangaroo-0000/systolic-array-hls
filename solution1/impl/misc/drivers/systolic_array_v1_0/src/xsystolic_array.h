// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.2 (64-bit)
// Tool Version Limit: 2023.10
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XSYSTOLIC_ARRAY_H
#define XSYSTOLIC_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xsystolic_array_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Control_BaseAddress;
} XSystolic_array_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XSystolic_array;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XSystolic_array_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XSystolic_array_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XSystolic_array_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XSystolic_array_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifdef SDT
int XSystolic_array_Initialize(XSystolic_array *InstancePtr, UINTPTR BaseAddress);
XSystolic_array_Config* XSystolic_array_LookupConfig(UINTPTR BaseAddress);
#else
int XSystolic_array_Initialize(XSystolic_array *InstancePtr, u16 DeviceId);
XSystolic_array_Config* XSystolic_array_LookupConfig(u16 DeviceId);
#endif
int XSystolic_array_CfgInitialize(XSystolic_array *InstancePtr, XSystolic_array_Config *ConfigPtr);
#else
int XSystolic_array_Initialize(XSystolic_array *InstancePtr, const char* InstanceName);
int XSystolic_array_Release(XSystolic_array *InstancePtr);
#endif

void XSystolic_array_Start(XSystolic_array *InstancePtr);
u32 XSystolic_array_IsDone(XSystolic_array *InstancePtr);
u32 XSystolic_array_IsIdle(XSystolic_array *InstancePtr);
u32 XSystolic_array_IsReady(XSystolic_array *InstancePtr);
void XSystolic_array_EnableAutoRestart(XSystolic_array *InstancePtr);
void XSystolic_array_DisableAutoRestart(XSystolic_array *InstancePtr);

void XSystolic_array_Set_accumulateMode(XSystolic_array *InstancePtr, u32 Data);
u32 XSystolic_array_Get_accumulateMode(XSystolic_array *InstancePtr);
void XSystolic_array_Set_totalCycles(XSystolic_array *InstancePtr, u32 Data);
u32 XSystolic_array_Get_totalCycles(XSystolic_array *InstancePtr);

void XSystolic_array_InterruptGlobalEnable(XSystolic_array *InstancePtr);
void XSystolic_array_InterruptGlobalDisable(XSystolic_array *InstancePtr);
void XSystolic_array_InterruptEnable(XSystolic_array *InstancePtr, u32 Mask);
void XSystolic_array_InterruptDisable(XSystolic_array *InstancePtr, u32 Mask);
void XSystolic_array_InterruptClear(XSystolic_array *InstancePtr, u32 Mask);
u32 XSystolic_array_InterruptGetEnabled(XSystolic_array *InstancePtr);
u32 XSystolic_array_InterruptGetStatus(XSystolic_array *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
