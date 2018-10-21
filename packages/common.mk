PACKAGE_INSTALL_DIR = $(abs_top_builddir)/build/install

ws_cflags = $(shell pkg-config --cflags $(PACKAGE_INSTALL_DIR)/lib/pkgconfig/libwebsockets.pc)
ws_ldflags = $(shell pkg-config --libs $(PACKAGE_INSTALL_DIR)/lib/pkgconfig/libwebsockets.pc)
