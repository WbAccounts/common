ROOT_DIR = ../../

INCLUDE += -I$(ROOT_DIR)/
INCLUDE += -I$(ROOT_DIR)/singleton/
INCLUDE += -I$(ROOT_DIR)/thread/
CPPFLAGS += $(INCLUDE)
CPPFLAGS += -pthread -g3 -std=c++11

# OBJS += $(ROOT_DIR)/singleton/singleton_class.o
# OBJS += $(ROOT_DIR)/singleton/singleton_template.o
OBJS += $(ROOT_DIR)/singleton/test/main.o
OBJS += $(ROOT_DIR)/singleton/test/mo.o

test : $(OBJS)
	$(CXX) $(CPPFLAGS) -o $@ $^ 

.PHONY : clean
clean:
	rm -f $(OBJS) test