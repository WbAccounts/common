INCLUDE += -I../
INCLUDE += -I../../
INCLUDE += -I../../log/
CPPFLAGS += $(INCLUDE)
CPPFLAGS += -g3 -pthread
CPPFLAGS += -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE

OBJS += main.o
OBJS += ../proc_monitor.o
OBJS += ../../log/log_helper.o

.PHONY : clean all
all : test monitor_test

test : $(OBJS)
	$(CXX) -o $@ $^ $(CPPFLAGS) 

monitor_test : monitor_test.o
	$(CXX) -o $@ $^

clean : 
	rm -f $(OBJS) test monitor_test.o monitor_test
