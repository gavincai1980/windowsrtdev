#
# Setup needed Tools and Libraries
#

ifeq "$(wildcard $(AP_WORK)\NWGNUcustom.ini)" "$(AP_WORK)\NWGNUcustom.ini"
include $(AP_WORK)\NWGNUcustom.ini
CUSTOM_INI = $(AP_WORK)\NWGNUcustom.ini
endif

ifndef VERBOSE
.SILENT:
endif

#
# Treat like an include
#
ifndef EnvironmentDefined

#
# simple macros for parsing makefiles
#
EOLIST:=
EMPTY :=
COMMA := ,
SPACE := $(EMPTY) $(EMPTY)

#
# Base environment
#

# Try and handle case issues
ifndef NOVELLLIBC
ifdef NovellLibC
NOVELLLIBC = $(NovellLibC)
endif
endif

ifndef NOVELLLIBC
NOVELLLIBC = C:/novell/ndk/libc
endif
ifneq "$(wildcard $(NOVELLLIBC)/include/ndkvers.h)" "$(NOVELLLIBC)/include/ndkvers.h"
$(error NOVELLLIBC does not point to a valid Novell LIBC SDK) 
endif

# This is a placeholder
# ifndef LDAPSDK
# LDAPSDK = C:/novell/ndk/cldapsdk/NetWare/libc
# endif
ifdef LDAPSDK
ifneq "$(wildcard $(LDAPSDK)/inc/ldap.h)" "$(LDAPSDK)/inc/ldap.h"
$(error LDAPSDK does not point to a valid Novell CLDAP SDK) 
endif
endif

ifndef PCRESRC
PCRESRC = $(AP_WORK)/srclib/pcre
endif
ifneq "$(wildcard $(PCRESRC)/pcre-config.in)" "$(PCRESRC)/pcre-config.in"
$(error PCRESRC does not point to a valid PCRE source tree) 
endif

# This is a placeholder
# ifndef ZLIBSDK
# ZLIBSDK = C:/novell/ndk/zlibsdk
# endif

ifndef METROWERKS
METROWERKS = $(ProgramFiles)\Metrowerks\CodeWarrior
endif

# If LM_LICENSE_FILE isn't defined, define a variable that can be used to
# restart make with it defined
ifndef LM_LICENSE_FILE
NO_LICENSE_FILE = NO_LICENSE_FILE
endif

#
# Set the Release type that you want to build, possible values are:
#
#  debug	- full debug switches are set
#  noopt	- normal switches are set (default)
#  optimized	- optimization switches are set

ifdef reltype
RELEASE=$(reltype)
endif

ifdef RELTYPE
RELEASE=$(RELTYPE)
endif

ifdef debug
RELEASE=debug
endif

ifdef DEBUG
RELEASE=debug
endif

ifdef optimized
RELEASE=optimized
endif

ifdef OPTIMIZED
RELEASE=optimized
endif

ifndef RELEASE
RELEASE = optimized
endif

ifeq "$(RELEASE)" "debug"
OBJDIR = Debug
endif

ifeq "$(RELEASE)" "noopt"
OBJDIR = Noopt
endif

ifeq "$(RELEASE)" "optimized"
OBJDIR = Release
endif

#
# Setup compiler information
#

# MetroWerks NLM tools
CC	= mwccnlm
CPP	= mwccnlm
LINK	= mwldnlm
LIB	= mwldnlm -type library -w nocmdline
WIN_CC	= mwcc

# Setup build tools
AWK	= awk

# Setup distribution tools
ZIP	= zip -qr9
7ZA	= 7za >NUL a

#
# Declare Command and tool macros here
#

# Os2LibPath is an extra check to see if we are on NT
ifdef Os2LibPath
OS = Windows_NT
endif

ifeq "$(OS)" "Windows_NT"
CMD = cmd /C
DEL = del /F
DELTREE = rd /s/q
WINNT = 1
XCOPYSW = /E
else
CMD = command /C
DEL = del
DELTREE = deltree /y
XCOPYSW = /E /Y
endif

CHK = $(CMD) if exist
CHKNOT = $(CMD) if not exist


NOVI	= $(NOVELLLIBC)/imports
PRELUDE	= $(NOVI)/libcpre.o

INCDIRS	= $(NOVELLLIBC)/include;
ifndef USE_STDSOCKETS
INCDIRS	+= $(NOVELLLIBC)/include/winsock;
endif
ifneq "$(LDAPSDK)" ""
INCDIRS += $(LDAPSDK)/inc;
endif
ifneq "$(ZLIBSDK)" ""
INCDIRS += $(ZLIBSDK);
endif
ifneq "$(PCRESRC)" ""
INCDIRS += $(PCRESRC);
endif

DEFINES	= -DNETWARE 
ifndef USE_STDSOCKETS
DEFINES	+= -DUSE_WINSOCK
endif
ifndef DEBUG
DEFINES	+= -DNDEBUG
endif

ifdef USE_STDSOCKETS
VERSION_SKT = (BSDSOCK)
else
VERSION_SKT = (WINSOCK)
endif

# MetroWerks static Libraries
CLIB3S	= $(METROWERKS)/Novell Support/Metrowerks Support/Libraries/Runtime/mwcrtl.lib
MATH3S	=
PLIB3S	= $(METROWERKS)/Novell Support/Metrowerks Support/Libraries/MSL C++/MWCPP.lib

ifeq "$(OS)" "Windows_NT"
# MetroWerks Win32 build flags to create build tools
MWCW_MSL         = "$(METROWERKS)/MSL"
MWCW_W32         = "$(METROWERKS)/Win32-x86 Support"
CC_FOR_BUILD     = $(WIN_CC)
CFLAGS_FOR_BUILD =  -O2 -gccinc -nodefaults -proc 586 -w off
CFLAGS_FOR_BUILD += -ir $(MWCW_MSL) -ir $(MWCW_W32) -lr $(MWCW_MSL) -lr $(MWCW_W32)
CFLAGS_FOR_BUILD += -lMSL_All_x86.lib -lkernel32.lib -luser32.lib
else
# GNUC build flags to create build tools
CC_FOR_BUILD     = gcc
CFLAGS_FOR_BUILD = -Wall -O2
endif

# Base compile flags
# and prefix or precompiled header added here.

# The default flags are as follows:
#
# -c                    compile only, no link
# -nosyspath            treat #include <...> like #include "..."
# -Cpp_exceptions off   disable C++ exceptions
# -RTTI off             disable C++ run-time typing information
# -align 4              align on 4 byte bounderies
# -w nocmdline          disable command-line driver/parser warnings
# -proc PII             generate code base on Pentium II instruction set
# -inst mmx             use MMX extensions (not used)

CFLAGS += -c -nosyspath -Cpp_exceptions off -RTTI off -align 4 -w nocmdline -proc PII

ifdef CC_MAX_ERRORS
CFLAGS += -maxerrors $(CC_MAX_ERRORS)
else
CFLAGS += -maxerrors 1
endif

ifeq "$(REQUIRE_PROTOTYPES)" "1"
CFLAGS += -r
endif

# -g                    generate debugging information
# -O0                   level 0 optimizations
ifeq "$(RELEASE)" "debug"
CFLAGS += -g -O0
endif

# -O4,p                 level 4 optimizations, optimize for speed
ifeq "$(RELEASE)" "optimized"
CFLAGS += -O4,p
endif

# -prefix pre_nw.h      #include pre_nw.h for all files
CFLAGS += -prefix pre_nw.h


PATH:=$(PATH);$(METROWERKS)\bin;$(METROWERKS)\Other Metrowerks Tools\Command Line Tools

#
# Declare major project deliverables output directories here
#

ifdef DEST
INSTALL = $(DEST)
ifeq (\, $(findstring \,$(INSTALL)))
INSTDIRS = $(DEST)
endif
endif

ifdef dest
INSTALL = $(dest)
ifeq (\, $(findstring \,$(INSTALL)))
INSTDIRS = $(dest)
endif
endif

ifndef INSTALL
INSTALL = $(AP_WORK)\Dist
INSTDIRS = $(AP_WORK)\Dist
endif

ifeq ($(MAKECMDGOALS),installdev)
ifndef BASEDIR
export BASEDIR = apache_$(VERSION_STR)-sdk
endif
else
ifndef BASEDIR
export BASEDIR = Apache2
endif
endif

ifndef PORT
PORT = 80
endif

ifndef SSLPORT
SSLPORT = 443
endif

# Add support for building IPV6 alongside
ifneq "$(IPV6)" ""
DEFINES += -DNW_BUILD_IPV6
# INCDIRS := $(NOVELLLIBC)/include/winsock/IPV6;$(INCDIRS)

ifneq "$(findstring IPV6,$(OBJDIR))" "IPV6"
OBJDIR := $(OBJDIR)_IPV6
endif
        
ifneq "$(findstring IPV6,$(INSTALL))" "IPV6"
INSTALL := $(INSTALL)_IPV6
endif        

ifneq "$(findstring IPV6,$(INSTDIRS))" "IPV6"
INSTDIRS := $(INSTDIRS)_IPV6
endif

endif

INSTDEVDIRS := \
	$(INSTDIRS) \
	$(INSTALL)\$(BASEDIR)\build \
	$(INSTALL)\$(BASEDIR)\include \
	$(INSTALL)\$(BASEDIR)\lib \
	$(EOLIST) 

INSTDIRS += \
	$(INSTALL)\$(BASEDIR) \
	$(INSTALL)\$(BASEDIR)\bin \
	$(INSTALL)\$(BASEDIR)\cgi-bin \
	$(INSTALL)\$(BASEDIR)\conf \
	$(INSTALL)\$(BASEDIR)\error \
	$(INSTALL)\$(BASEDIR)\htdocs \
	$(INSTALL)\$(BASEDIR)\icons \
	$(INSTALL)\$(BASEDIR)\logs \
	$(INSTALL)\$(BASEDIR)\man \
	$(INSTALL)\$(BASEDIR)\manual \
	$(INSTALL)\$(BASEDIR)\modules \
	$(EOLIST) 

#
# Common directories
#

SRC		= $(subst \,/,$(AP_WORK))
APR		= $(subst \,/,$(APR_WORK))
APRUTIL		= $(subst \,/,$(APU_WORK))
APBUILD		= $(SRC)/build
STDMOD		= $(SRC)/modules
HTTPD		= $(SRC)/modules/http
DAV		= $(SRC)/modules/dav
NWOS		= $(SRC)/os/netware
SERVER		= $(SRC)/server
SUPMOD		= $(SRC)/support
APULDAP		= $(APRUTIL)/ldap
XML		= $(APRUTIL)/xml
APRTEST		= $(APR)/test
PCRE		= $(PCRESRC)

#
# Internal Libraries
#

APRLIB		= $(APR)/$(OBJDIR)/aprlib.lib
APRUTLIB	= $(APRUTIL)/$(OBJDIR)/aprutil.lib
APULDAPLIB	= $(APULDAP)/$(OBJDIR)/apuldap.lib
STMODLIB	= $(STDMOD)/$(OBJDIR)/stdmod.lib
PCRELIB		= $(SRC)/$(OBJDIR)/pcre.lib
NWOSLIB		= $(NWOS)/$(OBJDIR)/netware.lib
SERVLIB		= $(SERVER)/$(OBJDIR)/server.lib
HTTPDLIB	= $(HTTPD)/$(OBJDIR)/httpd.lib
XMLLIB		= $(XML)/$(OBJDIR)/xmllib.lib

#
# Additional general defines
#

EnvironmentDefined = 1
endif # ifndef EnvironmentDefined

# This is always set so that it will show up in lower directories

ifdef Path
Path = $(PATH)
endif

