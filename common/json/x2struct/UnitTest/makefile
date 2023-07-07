CC=gcc
CXX=g++
AR=ar
DEBUG=1
###########################################################
CPPFLAGS += -D__QAXVERSION__="\"${VER}\""
CPPFLAGS += -D__REVISION__="\"${REVISION}\""

ifeq ($(DEBUG), 1)
	CPPFLAGS += -Werror -Wno-deprecated-declarations
else
	GCC_VERSION = $(shell gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
	ifeq ($(shell expr $(GCC_VERSION) \> 40102), 1)
		CPPFLAGS += -Werror -Wno-deprecated-declarations
	else
		CPPFLAGS += -Wall
	endif
endif

CPPFLAGS += -fPIC -pthread
CPPFLAGS += -ffunction-sections -fdata-sections -fvisibility=hidden -fno-strict-aliasing

ifeq ($(DEBUG), 1)
	CPPFLAGS += -g3 -DDEBUG -DTEST
else
	CPPFLAGS += -g3 -O3 -Os -DNDEBUG
	CPPFLAGS += -DSQLITE_HAS_CODEC
endif

##########################################################################
ROOTDIR = ../../..
COMMONDIR = ${ROOTDIR}/common
DEPENDLIBSDIR = ${ROOTDIR}/common/dependlibs

##########################################################################
INCPATH += -I${ROOTDIR}
INCPATH += -I${ROOTDIR}/common
INCPATH += -I${ROOTDIR}/common/ASFramework
INCPATH += -I${DEPENDLIBSDIR}
INCPATH += -I${DEPENDLIBSDIR}/libjsoncpp/include
INCPATH += -I${DEPENDLIBSDIR}/libjsoncpp/include/jsoncpp
CPPFLAGS += $(INCPATH)

##########################################################################
LIBS += -Wl,--as-needed -Wl,-Bsymbolic
LIBS += -Wl,--hash-style=sysv

LIBS += $(DEPENDLIBSDIR)/libjsoncpp/libjsoncpp.a
LIBS += ${DEPENDLIBSDIR}/sqlite3aes/libsqlite3.a
LIBS += ${DEPENDLIBSDIR}/libopensslnew/lib/libssl.a
LIBS += ${DEPENDLIBSDIR}/libopensslnew/lib/libcrypto.a
LIBS += ${DEPENDLIBSDIR}/minizip/libminizip.a
LIBS += ${DEPENDLIBSDIR}/libz/lib/libz.a
LIBS += -lpthread
LIBS += -ldl

##########################################################################
COMMON_OBJS += ${COMMONDIR}/ASFramework/util/ASLogImpl.o
COMMON_OBJS += ${COMMONDIR}/debug/time_debug.o
COMMON_OBJS += ${COMMONDIR}/debug/top_tools.o
COMMON_OBJS += ${COMMONDIR}/log/log.o
COMMON_OBJS += ${COMMONDIR}/utils/file_utils.o
COMMON_OBJS += ${COMMONDIR}/utils/proc_info_utils.o
COMMON_OBJS += ${COMMONDIR}/utils/system_utils.o
COMMON_OBJS += ${COMMONDIR}/qh_thread/thread.o

##########################################################################
TEST_OBJS += $(COMMONDIR)/x2struct/UnitTest/example_json.o

ALL_OBJS+=$(COMMON_OBJS) $(TEST_OBJS)
##########################################################################
TARGET = x2struct

##########################################################################
.PHONY:all
all: $(TARGET) check

$(TARGET) : $(ALL_OBJS)
	$(CXX) $(CPPFLAGS) -o $(TARGET) $^ $(LIBS)

check:
	nm $(TARGET) | grep -v "GLIB" | grep -v "CXX" | grep -v "GCC" | grep " U " || echo "make success."

.PHONY:clean
clean:
	rm -f $(TARGET) $(ALL_OBJS)