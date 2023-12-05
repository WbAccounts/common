CC=gcc
CXX=g++
AR=ar
DEBUG=1
GEN_PROTO=1
TEST=0
TAG=1
###########################################################
ifeq ($(TAG), 14)
	CPPFLAGS += -std=c++14 
else ifeq ($(TAG), 11)
	CPPFLAGS += -std=c++11
else
endif

ifeq ($(DEBUG), 1)
    CPPFLAGS += -g3
else
endif

##########################################################################
ROOTDIR = ../
##########################################################################
INCPATH += -I${ROOTDIR}
INCPATH += -I${ROOTDIR}/file_utils
INCPATH += -I${ROOTDIR}/units
INCPATH += -I${ROOTDIR}/log
CPPFLAGS += $(INCPATH)

##########################################################################
LIBS += -lrt -lpthread -ldl -lm

##########################################################################
COMMON_OBJS += ${ROOTDIR}/file_utils/file_utils.o


UNIT_FILE_OBJS += ${ROOTDIR}/units/unit_file.o

###########################################################
TARGET = unit_file
###########################################################
.PHONY:all
.PHONY: unit_file
all: $(TARGET)

unit_file: $(COMMON_OBJS) $(UNIT_FILE_OBJS)
	$(CXX) $(CPPFLAGS) $(COMMON_OBJS) $(UNIT_FILE_OBJS) -o $@ $(LIBS)
	make check1

check1:
	nm -u unit_file | grep -v "GLIB" | grep -v "CXX" | grep -v "GCC" | grep " U " || echo "make success."

.PHONY:clean
clean:
	rm -f $(TARGET) $(COMMON_OBJS) $(UNIT_FILE_OBJS)
