
#ifndef DMA_GUARD_H
#define DMA_GUARD_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"

#define DMA_GUARD_s_axi_control_SLV_REG0_OFFSET 0
#define DMA_GUARD_s_axi_control_SLV_REG1_OFFSET 4
#define DMA_GUARD_s_axi_control_SLV_REG2_OFFSET 8
#define DMA_GUARD_s_axi_control_SLV_REG3_OFFSET 12
#define DMA_GUARD_s_axi_control_SLV_REG4_OFFSET 16
#define DMA_GUARD_s_axi_control_SLV_REG5_OFFSET 20
#define DMA_GUARD_s_axi_control_SLV_REG6_OFFSET 24
#define DMA_GUARD_s_axi_control_SLV_REG7_OFFSET 28
#define DMA_GUARD_s_axi_control_SLV_REG8_OFFSET 32
#define DMA_GUARD_s_axi_control_SLV_REG9_OFFSET 36
#define DMA_GUARD_s_axi_control_SLV_REG10_OFFSET 40


/**************************** Type Definitions *****************************/
/**
 *
 * Write/Read 32 bit value to/from DMA_GUARD user logic memory (BRAM).
 *
 * @param   Address is the memory address of the DMA_GUARD device.
 * @param   Data is the value written to user logic memory.
 *
 * @return  The data from the user logic memory.
 *
 * @note
 * C-style signature:
 * 	void DMA_GUARD_mWriteMemory(u32 Address, u32 Data)
 * 	u32 DMA_GUARD_mReadMemory(u32 Address)
 *
 */
#define DMA_GUARD_mWriteMemory(Address, Data) \
    Xil_Out32(Address, (u32)(Data))
#define DMA_GUARD_mReadMemory(Address) \
    Xil_In32(Address)

/************************** Function Prototypes ****************************/
/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the DMA_GUARDinstance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
XStatus DMA_GUARD_Mem_SelfTest(void * baseaddr_p);

/**
 *
 * Write a value to a DMA_GUARD register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the DMA_GUARDdevice.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void DMA_GUARD_mWriteReg(u32 BaseAddress, unsigned RegOffset, u32 Data)
 *
 */
#define DMA_GUARD_mWriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/**
 *
 * Read a value from a DMA_GUARD register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the DMA_GUARD device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	u32 DMA_GUARD_mReadReg(u32 BaseAddress, unsigned RegOffset)
 *
 */
#define DMA_GUARD_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

/************************** Function Prototypes ****************************/
/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the DMA_GUARD instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
XStatus DMA_GUARD_Reg_SelfTest(void * baseaddr_p);

#endif // DMA_GUARD_H
