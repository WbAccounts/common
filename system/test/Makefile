INCLUDE := -I./
INCLUDE += -I../
INCLUDE += -I../../../
INCLUDE += -I../../../log/
CPPFLAGS := $(INCLUDE)
CFLAGS := $(CPPFLAGS)

# OBJS := ./test.o
# OBJS += ../system_utils.o

VPATH := ./
VPATH += ../
OBJS := test.o
OBJS += system_utils.o

all : test

test : $(OBJS)
	$(CXX) $(CPPFLAGS) -o $@ $^

.PHONY : clean all

clean :
	rm -f test; rm -f *.o
