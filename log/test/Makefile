INCLUDE += -I../
INCLUDE += -I../../

#如果不添加此宏定义，debug日志将不会输出，尽管你设置了debug的日志级别
CPPFLAGS += -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
CPPFLAGS += $(INCLUDE) -g3 -pthread 

OBJS += main.o
OBJS += ../log_helper.o

test : $(OBJS)
	$(CXX) -o $@ $^ $(CPPFLAGS)

clean :
	rm -f $(OBJS) test

.PHONY : clean