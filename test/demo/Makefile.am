include ../../packages/common.mk

bin_PROGRAMS = SimpleWS

SimpleWS_SOURCES = SimpleWS.cpp
SimpleWS_LDADD = $(builddir)/../../lib/libhttp.la
SimpleWS_CXXFLAGS = -I$(srcdir)/../../include $(ws_cflags) -DROOT=\"$(abs_srcdir)\"
SimpleWS_LDFLAGS = $(ws_ldflags)
