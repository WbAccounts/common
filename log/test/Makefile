INCLUDE += -I../
INCLUDE += -I../../

CPPFLAGS += $(INCLUDE) -g3 -pthread

OBJS += main.o
OBJS += ../log_helper.o

test : $(OBJS)
	$(CXX) -o $@ $^ $(CPPFLAGS)

clean :
	rm -f $(OBJS) test

.PHONY : clean