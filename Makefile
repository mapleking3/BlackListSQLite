APP			:= test
all:		$(APP)

CC			:= $(shell which gcc)
CXX			:= $(shell which g++)
AR			:= $(shell which ar)
LD			:= $(shell which ld)

CFLAGS		+= -Wall -W -O2

LD_FLAGS	+= -lpthread -lsqlite3 -lrt

INC_FLAG    += 

APP_ROOT	:= $(shell pwd)

export CC CXX AR LD CFLAGS INC_FLAG LD_FLAGS 

CSRCS		:= test.c plate_bw_list.c
CXXSRCS		:= 

CXXOBJS		:= $(CXXSRCS:.cpp=.o)
COBJS		:= $(CSRCS:.c=.o)
OBJS		:= $(CXXOBJS) $(COBJS)

DEPS		:= $(OBJS:.o=.d)

%.o : %.c
	$(CC) $(CFLAGS) $(INC_FLAG) -c $< -o $@
%.d : %.c
	@set -e;$(CC) $(CFLAGS) $(INC_FLAG) $(CPPFLAGS) -MM $< | sed -e 's/$(basename $@).o/$(basename $@).o $(basename $@).d/' > $@
%.o : %.cpp
	$(CXX) $(CFLAGS) $(INC_FLAG) -c $< -o $@
%.d : %.cpp
	@set -e;$(CXX) $(CFLAGS) $(INC_FLAG) $(CPPFLAGS) -MM $< | sed -e 's/$(basename $@).o/$(basename $@).o $(basename $@).d/' > $@

.PHONY:	$(APP)

$(APP) : $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LD_FLAGS)

clean:
	$(RM) -f $(OBJS) $(DEPS) $(APP) $(MODDEP)

sinclude $(DEPS)
