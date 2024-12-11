INCLUDE += -I../
INCLUDE += -I../../
INCLUDE += -I../../log/
CPPFLAGS += $(INCLUDE) -g3 -pthread
CPPFLAGS += -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE

OBJS += main.o
OBJS += ../../log/log_helper.o

test : $(OBJS)
	$(CXX) -o $@ $^ $(CPPFLAGS)

.PHONY : clean
clean :
	rm -f $(OBJS) test