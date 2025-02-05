// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.2 (64-bit)
// Tool Version Limit: 2023.10
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xsystolic_array.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XSystolic_array_CfgInitialize(XSystolic_array *InstancePtr, XSystolic_array_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XSystolic_array_Start(XSystolic_array *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_AP_CTRL) & 0x80;
    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XSystolic_array_IsDone(XSystolic_array *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XSystolic_array_IsIdle(XSystolic_array *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XSystolic_array_IsReady(XSystolic_array *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XSystolic_array_EnableAutoRestart(XSystolic_array *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XSystolic_array_DisableAutoRestart(XSystolic_array *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_AP_CTRL, 0);
}

void XSystolic_array_Set_accumulateMode(XSystolic_array *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_ACCUMULATEMODE_DATA, Data);
}

u32 XSystolic_array_Get_accumulateMode(XSystolic_array *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_ACCUMULATEMODE_DATA);
    return Data;
}

void XSystolic_array_Set_totalCycles(XSystolic_array *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_TOTALCYCLES_DATA, Data);
}

u32 XSystolic_array_Get_totalCycles(XSystolic_array *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_TOTALCYCLES_DATA);
    return Data;
}

void XSystolic_array_InterruptGlobalEnable(XSystolic_array *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_GIE, 1);
}

void XSystolic_array_InterruptGlobalDisable(XSystolic_array *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_GIE, 0);
}

void XSystolic_array_InterruptEnable(XSystolic_array *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_IER);
    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_IER, Register | Mask);
}

void XSystolic_array_InterruptDisable(XSystolic_array *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_IER);
    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_IER, Register & (~Mask));
}

void XSystolic_array_InterruptClear(XSystolic_array *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSystolic_array_WriteReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_ISR, Mask);
}

u32 XSystolic_array_InterruptGetEnabled(XSystolic_array *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_IER);
}

u32 XSystolic_array_InterruptGetStatus(XSystolic_array *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSystolic_array_ReadReg(InstancePtr->Control_BaseAddress, XSYSTOLIC_ARRAY_CONTROL_ADDR_ISR);
}

