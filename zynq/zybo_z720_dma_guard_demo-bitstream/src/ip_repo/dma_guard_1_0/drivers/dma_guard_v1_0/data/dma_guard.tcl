

proc generate {drv_handle} {
	xdefine_include_file $drv_handle "xparameters.h" "dma_guard" "NUM_INSTANCES" "DEVICE_ID"  "C_s_axi_control_BASEADDR" "C_s_axi_control_HIGHADDR" "C_S00_AXI_BASEADDR" "C_S00_AXI_HIGHADDR"
}
