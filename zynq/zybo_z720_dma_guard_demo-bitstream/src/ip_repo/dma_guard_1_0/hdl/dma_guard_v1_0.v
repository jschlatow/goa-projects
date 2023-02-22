
`timescale 1 ns / 1 ps

	module dma_guard_v1_0 #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Master Bus Interface M00_AXI
		parameter integer C_M00_AXI_ID_WIDTH	= 1,
		parameter integer C_M00_AXI_ADDR_WIDTH	= 32,
		parameter integer C_M00_AXI_DATA_WIDTH	= 32,
		parameter integer C_M00_AXI_AWUSER_WIDTH	= 0,
		parameter integer C_M00_AXI_ARUSER_WIDTH	= 0,
		parameter integer C_M00_AXI_WUSER_WIDTH	= 0,
		parameter integer C_M00_AXI_RUSER_WIDTH	= 0,
		parameter integer C_M00_AXI_BUSER_WIDTH	= 0,

		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_ID_WIDTH	= 1,
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 32,
		parameter integer C_S00_AXI_AWUSER_WIDTH	= 0,
		parameter integer C_S00_AXI_ARUSER_WIDTH	= 0,
		parameter integer C_S00_AXI_WUSER_WIDTH	= 0,
		parameter integer C_S00_AXI_RUSER_WIDTH	= 0,
		parameter integer C_S00_AXI_BUSER_WIDTH	= 0,

		// Parameters of Axi Slave Bus Interface s_axi_control
		parameter integer C_s_axi_control_DATA_WIDTH	= 32,
		parameter integer C_s_axi_control_ADDR_WIDTH	= 6
	)
	(
		// Users to add ports here
		
		input wire  axi_aclk,
		input wire  axi_aresetn,

		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Master Bus Interface M00_AXI
		output wire [C_M00_AXI_ID_WIDTH-1 : 0] m00_axi_awid,
		output wire [C_M00_AXI_ADDR_WIDTH-1 : 0] m00_axi_awaddr,
		output wire [7 : 0] m00_axi_awlen,
		output wire [2 : 0] m00_axi_awsize,
		output wire [1 : 0] m00_axi_awburst,
		output wire  m00_axi_awlock,
		output wire [3 : 0] m00_axi_awcache,
		output wire [2 : 0] m00_axi_awprot,
		output wire [3 : 0] m00_axi_awqos,
		output wire [C_M00_AXI_AWUSER_WIDTH-1 : 0] m00_axi_awuser,
		output wire  m00_axi_awvalid,
		input wire  m00_axi_awready,
		output wire [C_M00_AXI_DATA_WIDTH-1 : 0] m00_axi_wdata,
		output wire [C_M00_AXI_DATA_WIDTH/8-1 : 0] m00_axi_wstrb,
		output wire  m00_axi_wlast,
		output wire [C_M00_AXI_WUSER_WIDTH-1 : 0] m00_axi_wuser,
		output wire  m00_axi_wvalid,
		input wire  m00_axi_wready,
		input wire [C_M00_AXI_ID_WIDTH-1 : 0] m00_axi_bid,
		input wire [1 : 0] m00_axi_bresp,
		input wire [C_M00_AXI_BUSER_WIDTH-1 : 0] m00_axi_buser,
		input wire  m00_axi_bvalid,
		output wire  m00_axi_bready,
		output wire [C_M00_AXI_ID_WIDTH-1 : 0] m00_axi_arid,
		output wire [C_M00_AXI_ADDR_WIDTH-1 : 0] m00_axi_araddr,
		output wire [7 : 0] m00_axi_arlen,
		output wire [2 : 0] m00_axi_arsize,
		output wire [1 : 0] m00_axi_arburst,
		output wire  m00_axi_arlock,
		output wire [3 : 0] m00_axi_arcache,
		output wire [2 : 0] m00_axi_arprot,
		output wire [3 : 0] m00_axi_arqos,
		output wire [C_M00_AXI_ARUSER_WIDTH-1 : 0] m00_axi_aruser,
		output wire  m00_axi_arvalid,
		input wire  m00_axi_arready,
		input wire [C_M00_AXI_ID_WIDTH-1 : 0] m00_axi_rid,
		input wire [C_M00_AXI_DATA_WIDTH-1 : 0] m00_axi_rdata,
		input wire [1 : 0] m00_axi_rresp,
		input wire  m00_axi_rlast,
		input wire [C_M00_AXI_RUSER_WIDTH-1 : 0] m00_axi_ruser,
		input wire  m00_axi_rvalid,
		output wire  m00_axi_rready,

		// Ports of Axi Slave Bus Interface S00_AXI
		input wire [C_S00_AXI_ID_WIDTH-1 : 0] s00_axi_awid,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
		input wire [7 : 0] s00_axi_awlen,
		input wire [2 : 0] s00_axi_awsize,
		input wire [1 : 0] s00_axi_awburst,
		input wire  s00_axi_awlock,
		input wire [3 : 0] s00_axi_awcache,
		input wire [2 : 0] s00_axi_awprot,
		input wire [3 : 0] s00_axi_awqos,
		input wire [3 : 0] s00_axi_awregion,
		input wire [C_S00_AXI_AWUSER_WIDTH-1 : 0] s00_axi_awuser,
		input wire  s00_axi_awvalid,
		output wire  s00_axi_awready,
		input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
		input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
		input wire  s00_axi_wlast,
		input wire [C_S00_AXI_WUSER_WIDTH-1 : 0] s00_axi_wuser,
		input wire  s00_axi_wvalid,
		output wire  s00_axi_wready,
		output wire [C_S00_AXI_ID_WIDTH-1 : 0] s00_axi_bid,
		output wire [1 : 0] s00_axi_bresp,
		output wire [C_S00_AXI_BUSER_WIDTH-1 : 0] s00_axi_buser,
		output wire  s00_axi_bvalid,
		input wire  s00_axi_bready,
		input wire [C_S00_AXI_ID_WIDTH-1 : 0] s00_axi_arid,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
		input wire [7 : 0] s00_axi_arlen,
		input wire [2 : 0] s00_axi_arsize,
		input wire [1 : 0] s00_axi_arburst,
		input wire  s00_axi_arlock,
		input wire [3 : 0] s00_axi_arcache,
		input wire [2 : 0] s00_axi_arprot,
		input wire [3 : 0] s00_axi_arqos,
		input wire [3 : 0] s00_axi_arregion,
		input wire [C_S00_AXI_ARUSER_WIDTH-1 : 0] s00_axi_aruser,
		input wire  s00_axi_arvalid,
		output wire  s00_axi_arready,
		output wire [C_S00_AXI_ID_WIDTH-1 : 0] s00_axi_rid,
		output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
		output wire [1 : 0] s00_axi_rresp,
		output wire  s00_axi_rlast,
		output wire [C_S00_AXI_RUSER_WIDTH-1 : 0] s00_axi_ruser,
		output wire  s00_axi_rvalid,
		input wire  s00_axi_rready,

		// Ports of Axi Slave Bus Interface s_axi_control
		input wire  s_axi_control_aclk,
		input wire  s_axi_control_aresetn,
		input wire [C_s_axi_control_ADDR_WIDTH-1 : 0] s_axi_control_awaddr,
		input wire [2 : 0] s_axi_control_awprot,
		input wire  s_axi_control_awvalid,
		output wire  s_axi_control_awready,
		input wire [C_s_axi_control_DATA_WIDTH-1 : 0] s_axi_control_wdata,
		input wire [(C_s_axi_control_DATA_WIDTH/8)-1 : 0] s_axi_control_wstrb,
		input wire  s_axi_control_wvalid,
		output wire  s_axi_control_wready,
		output wire [1 : 0] s_axi_control_bresp,
		output wire  s_axi_control_bvalid,
		input wire  s_axi_control_bready,
		input wire [C_s_axi_control_ADDR_WIDTH-1 : 0] s_axi_control_araddr,
		input wire [2 : 0] s_axi_control_arprot,
		input wire  s_axi_control_arvalid,
		output wire  s_axi_control_arready,
		output wire [C_s_axi_control_DATA_WIDTH-1 : 0] s_axi_control_rdata,
		output wire [1 : 0] s_axi_control_rresp,
		output wire  s_axi_control_rvalid,
		input wire  s_axi_control_rready
	);
	
	// signals passed to s_axi_control module
	wire ar_allowed;
	wire aw_allowed;
	wire [14 : 0] rd_len;
	wire [14 : 0] wr_len;
	
	assign rd_len = s00_axi_arlen << s00_axi_arsize;
	assign wr_len = s00_axi_awlen << s00_axi_awsize;

// Instantiation of Axi Bus Interface s_axi_control
	dma_guard_v1_0_s_axi_control # ( 
		.C_S_AXI_DATA_WIDTH(C_s_axi_control_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_s_axi_control_ADDR_WIDTH)
	) dma_guard_v1_0_s_axi_control_inst (
		.rd_allowed(ar_allowed),
		.rd_addr(s00_axi_araddr),
		.rd_len(rd_len),
		.wr_allowed(aw_allowed),
		.wr_addr(s00_axi_awaddr),
		.wr_len(wr_len),
		.S_AXI_ACLK(s_axi_control_aclk),
		.S_AXI_ARESETN(s_axi_control_aresetn),
		.S_AXI_AWADDR(s_axi_control_awaddr),
		.S_AXI_AWPROT(s_axi_control_awprot),
		.S_AXI_AWVALID(s_axi_control_awvalid),
		.S_AXI_AWREADY(s_axi_control_awready),
		.S_AXI_WDATA(s_axi_control_wdata),
		.S_AXI_WSTRB(s_axi_control_wstrb),
		.S_AXI_WVALID(s_axi_control_wvalid),
		.S_AXI_WREADY(s_axi_control_wready),
		.S_AXI_BRESP(s_axi_control_bresp),
		.S_AXI_BVALID(s_axi_control_bvalid),
		.S_AXI_BREADY(s_axi_control_bready),
		.S_AXI_ARADDR(s_axi_control_araddr),
		.S_AXI_ARPROT(s_axi_control_arprot),
		.S_AXI_ARVALID(s_axi_control_arvalid),
		.S_AXI_ARREADY(s_axi_control_arready),
		.S_AXI_RDATA(s_axi_control_rdata),
		.S_AXI_RRESP(s_axi_control_rresp),
		.S_AXI_RVALID(s_axi_control_rvalid),
		.S_AXI_RREADY(s_axi_control_rready)
	);

	// Add user logic here 
	
	// signals for read-transfer emulation/interception
	reg axi_arvalid;
	reg axi_arready;
	reg axi_rvalid;
	reg axi_rresp;
	reg axi_rlast;
	reg [1:0] read_state;
	
	// signals for write-transfer emulation/interception
	reg axi_awvalid;
	reg axi_awready;
	reg axi_wvalid;
	reg axi_wready;
	reg axi_bresp;
	reg axi_bvalid;
	reg [1:0] write_state;

	// read/write state values
	localparam [1:0]
	   FORWARD = 2'b00,
	   READY   = 2'b01,
	   WAIT    = 2'b10,
	   RESP    = 2'b11;
	
	// read response values
	localparam [1:0]
	   OKAY    = 2'b00,
	   EXOKAY  = 2'b01,
	   SLVERR  = 2'b10,
	   DECERR  = 2'b11;

	assign m00_axi_awid      = s00_axi_awid;
	assign m00_axi_awaddr    = s00_axi_awaddr;
	assign m00_axi_awlen     = s00_axi_awlen;
	assign m00_axi_awsize    = s00_axi_awsize;
	assign m00_axi_awburst   = s00_axi_awburst;
	assign m00_axi_awlock    = s00_axi_awlock;
	assign m00_axi_awcache   = s00_axi_awcache;
	assign m00_axi_awprot    = s00_axi_awprot;
	assign m00_axi_awqos     = s00_axi_awqos;
	assign m00_axi_awuser    = s00_axi_awuser;
	assign m00_axi_awvalid   = write_state == FORWARD && aw_allowed ? s00_axi_awvalid : axi_awvalid;
	assign s00_axi_awready   = write_state == FORWARD && aw_allowed ? m00_axi_awready : axi_awready;
	assign m00_axi_wdata     = s00_axi_wdata;
	assign m00_axi_wstrb     = s00_axi_wstrb;
	assign m00_axi_wlast     = s00_axi_wlast;
	assign m00_axi_wuser     = s00_axi_wuser;
	assign m00_axi_wvalid    = write_state == FORWARD && aw_allowed ? s00_axi_wvalid : axi_wvalid;
	assign s00_axi_wready    = write_state == FORWARD && aw_allowed ? m00_axi_wready : axi_wready;
	assign s00_axi_bid       = m00_axi_bid;
	assign s00_axi_bresp     = write_state == RESP ? axi_bresp : m00_axi_bresp;
	assign s00_axi_buser     = m00_axi_buser;
	assign s00_axi_bvalid    = write_state == RESP ? axi_bvalid : m00_axi_bvalid;
	assign m00_axi_bready    = s00_axi_bready;

	assign m00_axi_arid      = s00_axi_arid;
	assign m00_axi_araddr    = s00_axi_araddr;
	assign m00_axi_arlen     = s00_axi_arlen;
	assign m00_axi_arsize    = s00_axi_arsize;
	assign m00_axi_arburst   = s00_axi_arburst;
	assign m00_axi_arlock    = s00_axi_arlock;
	assign m00_axi_arcache   = s00_axi_arcache;
	assign m00_axi_arprot    = s00_axi_arprot;
	assign m00_axi_arqos     = s00_axi_arqos;
	assign m00_axi_aruser    = s00_axi_aruser;
	assign m00_axi_arvalid   = read_state == FORWARD && ar_allowed == 1 ? s00_axi_arvalid : axi_arvalid;
	assign s00_axi_arready   = read_state == FORWARD && ar_allowed == 1 ? m00_axi_arready : axi_arready;
	assign s00_axi_rid       = m00_axi_rid;
	assign s00_axi_rdata     = m00_axi_rdata;
	assign s00_axi_rresp     = read_state == RESP ? axi_rresp : m00_axi_rresp;
	assign s00_axi_rlast     = read_state == RESP ? axi_rlast : m00_axi_rlast;
	assign s00_axi_ruser     = m00_axi_ruser;
	assign s00_axi_rvalid    = read_state == RESP ? axi_rvalid : m00_axi_rvalid;
	assign m00_axi_rready    = s00_axi_rready;

	// implement clock-synchronous write signal updates
	always @(posedge axi_aclk)
	begin
		if (axi_aresetn == 1'b0)
			begin
				write_state  <= FORWARD;
				axi_awvalid <= 1'b0;
				axi_awready <= 1'b0;
				axi_wvalid  <= 1'b0;
				axi_wready  <= 1'b0;
				axi_bresp   <= 1'b0;
				axi_bvalid  <= 1'b0;
			end
		else
			begin
				case (write_state)
					READY: begin
						if (axi_awready && s00_axi_awvalid)
							begin
								// deassert ready signal to stop accepting write requests
								axi_awready <= 1'b0;
								// assert the wready signal to accept data
								axi_wready <= 1'b1;
								write_state <= WAIT;
							end
					end
					WAIT: begin
						// wait for last wdata
						if (s00_axi_wvalid && s00_axi_wlast)
							begin
								write_state <= RESP;
								axi_wready <= 1'b0;
								axi_bvalid <= 1'b0;
							end
					end
					RESP: begin
						// insert our own response
						if (~axi_bvalid)
							begin
								axi_bresp  <= DECERR;
								axi_bvalid <= 1'b1;
							end
						else if (s00_axi_bready)
							begin
								axi_bvalid <= 1'b0;
								axi_bresp  <= 1'b0;
								write_state <= FORWARD;
							end
					end
					FORWARD: begin
						// master tries issueing an invalid transfer
						if (~aw_allowed && s00_axi_awvalid)
							begin
								// don't forward valid signal because we are blocking the transfer
								axi_awvalid <= 1'b0;
								// yet, accept address
								axi_awready <= 1'b1;
								write_state <= READY;
							end
						else
							begin
								axi_awvalid <= 1'b0;
								axi_awready <= 1'b0;
								axi_wvalid  <= 1'b0;
								axi_wready  <= 1'b0;
								axi_bresp   <= 1'b0;
								axi_bvalid  <= 1'b0;
							end
					end
					default: begin
						write_state <= FORWARD;
					end
				endcase
			end
	end
	
	// implement clock-synchronous read signal updates
	always @(posedge axi_aclk)
	begin
		if (axi_aresetn == 1'b0)
			begin
				read_state  <= FORWARD;
				axi_arvalid <= 1'b0;
				axi_arready <= 1'b0;
				axi_rvalid  <= 1'b0;
				axi_rresp   <= 1'b0;
				axi_rlast   <= 1'b0;
			end
		else
			begin
				case (read_state)
					READY: begin
						if (axi_arready && s00_axi_arvalid)
							begin
								// deassert ready signal to finish read request 
								axi_arready <= 1'b0;
								read_state <= WAIT;
							end
					end
					WAIT: begin
						// wait for m00 read channel to become silent
						if (~m00_axi_rvalid)
							begin
								read_state <= RESP;
								axi_rvalid <= 1'b0;
							end
					end
					RESP: begin
						// insert our own response
						if (~axi_rvalid)
							begin
								axi_rresp  <= DECERR;
								axi_rvalid <= 1'b1;
								axi_rlast  <= 1'b1;
							end
						else if (s00_axi_rready)
							begin
								axi_rvalid <= 1'b0;
								axi_rlast  <= 1'b0;
								read_state <= FORWARD;
							end
					end
					FORWARD: begin
						// master tries issueing an invalid transfer
						if (~ar_allowed && s00_axi_arvalid)
							begin
								// don't forward valid signal because we are blocking the transfer
								axi_arvalid <= 1'b0;
								// yet, accept address
								axi_arready <= 1'b1;
								read_state <= READY;
							end
						else
							begin
								axi_arvalid <= 1'b0;
								axi_arready <= 1'b0;
								axi_rvalid  <= 1'b0;
								axi_rresp   <= 1'b0;
								axi_rlast   <= 1'b0;
							end
					end
					default: begin
						read_state <= FORWARD;
					end
				endcase
			end
	end
	
	// User logic ends

	endmodule
