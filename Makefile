PREFIX      := arm-wrs-linux-gnueabi-armv6jel_vfp-uclibc_small-
#TARGET      := ../../libs/libbwlist.a
TARGET		:= libbwlist.a

CC			:= $(PREFIX)gcc
CXX			:= $(PREFIX)g++
AR			:= $(PREFIX)ar
LD			:= $(PREFIX)ld

CFLAGS		+= -Wall -W -O2 -g

INC_FLAG	+= -I.
LD_FLAGS	+= -L../urmv/libs
LD_FLAGS	+= -lrt -lsqlite3 -ldl -lpthread

CSRCS	:= bwlist.c
CXXSRCS	:=

COBJS	:= $(CSRCS:.c=.o)
CXXOBJS	:= $(CXXSRCS:.cpp=.o)
OBJS	:= $(COBJS) $(CXXOBJS)
DEPS	:= $(OBJS:.o=.d)

all:	$(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@
%.d : %.c
	@set -e; $(CC) $(CFLAGS) $(INC_FLAGS) -MM $< | sed -e 's/$(basename $@).o/$(basename $@).o $(basename $@).d/' > $@
%.o : %.cpp
	$(CXX) $(CFLAGS) $(INC_FLAGS) -c $< -o $@
%.d : %.cpp
	@set -e; $(CXX) $(CFLAGS) $(INC_FLAGS) -MM $< | sed -e 's/$(basename $@).o/$(basename $@).o $(basename $@).d/' > $@

$(TARGET) : $(OBJS)
	$(AR) r $@ $^

clean:
	$(RM) $(OBJS) $(DEPS) $(TARGET)

sinclude $(DEPS)
