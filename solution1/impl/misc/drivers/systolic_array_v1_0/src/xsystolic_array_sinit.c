// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.2 (64-bit)
// Tool Version Limit: 2023.10
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#ifdef SDT
#include "xparameters.h"
#endif
#include "xsystolic_array.h"

extern XSystolic_array_Config XSystolic_array_ConfigTable[];

#ifdef SDT
XSystolic_array_Config *XSystolic_array_LookupConfig(UINTPTR BaseAddress) {
	XSystolic_array_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XSystolic_array_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XSystolic_array_ConfigTable[Index].Control_BaseAddress == BaseAddress) {
			ConfigPtr = &XSystolic_array_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XSystolic_array_Initialize(XSystolic_array *InstancePtr, UINTPTR BaseAddress) {
	XSystolic_array_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XSystolic_array_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XSystolic_array_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XSystolic_array_Config *XSystolic_array_LookupConfig(u16 DeviceId) {
	XSystolic_array_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XSYSTOLIC_ARRAY_NUM_INSTANCES; Index++) {
		if (XSystolic_array_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XSystolic_array_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XSystolic_array_Initialize(XSystolic_array *InstancePtr, u16 DeviceId) {
	XSystolic_array_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XSystolic_array_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XSystolic_array_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

