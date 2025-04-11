LIB_OBJ := $(addsuffix .o,$(basename $(notdir $(LIB_SRC))))
VPATH = $(subst $(eval ) ,:,$(dir $(LIB_SRC)))

vnc_server: $(LIB_OBJ) vnc_server.o

vnc_server.o: vnc_server.cc vnc_output.h keycode_from_keysym.h Makefile
