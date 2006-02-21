AM_CPPFLAGS = \
	-DSCIM_PRIME_LOCALEDIR=\"$(datadir)/locale\" \
	-DSCIM_ICONDIR=\"@SCIM_ICONDIR@\"
INCLUDE	= \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src 

noinst_HEADERS = \
	intl.h \
	prime_commands.h \
	prime_connection.h \
	prime_session.h \
	scim_prime_action.h \
	scim_prime_imengine_factory.h \
	scim_prime_imengine.h \
	scim_prime_prefs.h

moduledir = @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES = prime.la

prime_la_SOURCES = \
	prime_connection.cpp \
	prime_session.cpp \
	scim_prime_action.cpp \
	scim_prime_imengine_factory.cpp \
	scim_prime_imengine.cpp

prime_la_CFLAGS = @SCIM_CFLAGS@  @SCIM_DEBUG_FLAGS@
prime_la_CXXFLAGS = @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

prime_la_LDFLAGS = \
	-avoid-version \
	-rpath $(moduledir) \
	-module \
	@LIBTOOL_EXPORT_OPTIONS@ \
	@SCIM_LIBS@

if SCIM_BUILD_SETUP
SETUP_MODULE = prime-imengine-setup.la
endif

setupmoduledir           = @SCIM_MODULEDIR@/SetupUI
setupmodule_LTLIBRARIES  = $(SETUP_MODULE)

prime_imengine_setup_la_SOURCES = \
	scim_prime_imengine_setup.cpp \
	scim_prime_prefs.cpp

prime_imengine_setup_la_CFLAGS  = @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

prime_imengine_setup_la_CXXFLAGS= @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

prime_imengine_setup_la_LDFLAGS = -avoid-version \
                                   -rpath $(setupmoduledir) \
                                   -module \
                                   @LIBTOOL_EXPORT_OPTIONS@ \
                                   @SCIM_GTKUTILS_LIBS@ \
                                   @INTLLIBS@ \
                                   @SCIM_LIBS@
