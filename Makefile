PREFIX      := 
APP			:= test_sqlite3_pc

all:		$(APP)

CC			:= $(PREFIX)gcc
CXX			:= $(PREFIX)g++
AR			:= $(PREFIX)ar
LD			:= $(PREFIX)ld

APP_LIB     := libblacklist

CFLAGS		+= -Wall -W -O2
CFLAGS      += -g
APP_ROOT	:= $(shell pwd)

LD_FLAGS	+= -L$(APP_ROOT)/lib -L$(APP_ROOT)/src

ifeq ($(CC), gcc)
LD_FLAGS	+= -lblacklist -lrt -lsqlite3 -ldl -lpthread
else
LD_FLAGS	+= -lblacklist -lrt -lEsqlite3 -ldl -lpthread
endif

INC_FLAG    += -I$(APP_ROOT)/include

export CC CXX AR LD CFLAGS INC_FLAG LD_FLAGS 


CSRCS		:= test.c
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

.PHONY:	$(APP) $(APP_LIB) clean

$(APP_LIB) :
	$(MAKE) -C src TARGET=$(APP_LIB)

$(APP) : $(OBJS) $(APP_LIB)
#$(CXX) $(OBJS) src/$(APP_LIB).a -o $@ $(LD_FLAGS)
	$(CXX) $(OBJS) -o $@ $(LD_FLAGS)

clean:
	$(RM) -f $(OBJS) $(DEPS) $(APP) $(MODDEP)

distclean:
	$(MAKE) -C src TARGET=$(APP_LIB) clean

sinclude $(DEPS)
