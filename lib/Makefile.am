include ../packages/common.mk

noinst_LTLIBRARIES = libhttp.la
libhttp_la_SOURCES = server/WebServer.cpp server/WebServerImpl.cpp server/WSServerImpl.cpp \
                     server/WebSocketHandler.cpp server/WebSocketRequest.cpp
libhttp_la_CXXFLAGS = -I$(srcdir)/../include $(ws_cflags)
