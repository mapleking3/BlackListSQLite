APP		:= exe

CC      := $(shell which gcc)
CXX     := $(shell which g++)
AR      := $(shell which ar)
LD      := $(shell which ld)

CFLAGS	+= -g -Wall -O2
LD_FLAG += -lrt -lsqlite3 -pthread -L. 

CSCRS	:= test.c plate_bw_list.c
CXXSCRS := 
COBJS	:= $(CSCRS:.c=.o)
CXXOBJS := $(CXXSCRS:.cpp=.o)
OBJS	:= $(COBJS) $(CXXOBJS)


LIBS	:= 

%.o:%.c 
	$(CC) $(CFLAGS) -c $< -o $@
%.o:%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@
$(APP):$(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $@ $(LD_FLAG)  

.PHONY: clean
clean:
	-@rm -rf $(COBJS) $(APP) test.db
