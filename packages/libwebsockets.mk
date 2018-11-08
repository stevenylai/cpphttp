WS_VERSION=v2.0.0
WS_DIR = $(abs_top_srcdir)/packages/libwebsockets
WS_HEADER = $(PACKAGE_INSTALL_DIR)/include/libwebsockets.h
WS_LIB = $(PACKAGE_INSTALL_DIR)/lib/pkgconfig/libwebsockets.pc
WS_TARGET = $(WS_LIB)

ws-all: $(WS_TARGET)

$(WS_TARGET):
	$(MKDIR_P) $(WS_DIR)/build
	cd $(WS_DIR)/build && $(CMAKE) .. -DCMAKE_C_FLAGS=-g -DCMAKE_C_COMPILER=$(CC) -DCMAKE_INSTALL_PREFIX:PATH=$(PACKAGE_INSTALL_DIR)
	$(MAKE) $(MAKE_OPTIONS) -C $(WS_DIR)/build  && $(MAKE) $(MAKE_OPTIONS) -C $(WS_DIR)/build install

ws-clean:
	if [ -d $(WS_DIR)/build ]; then \
		cd $(WS_DIR)/build && $(MAKE) clean; \
		rm -Rf $(WS_HEADER); \
		rm -Rf $(WS_LIB); \
	fi
