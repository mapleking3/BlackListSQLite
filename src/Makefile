CSRCS	:= plate_bw_list.c
CXXSRCS	:=

COBJS	:= $(CSRCS:.c=.o)
CXXOBJS	:= $(CXXSRCS:.cpp=.o)
OBJS	:= $(COBJS) $(CXXOBJS)
DEPS	:= $(OBJS:.o=.d)

LIB		:= $(TARGET).a
all:	$(LIB)

%.o : %.c
	$(CC) $(CFLAGS) $(INC_FLAG) -c $< -o $@
%.d : %.c
	@set -e; $(CC) $(CFLAGS) $(INC_FLAG) $(CPPFLAGS) -MM $< | sed -e 's/$(basename $@).o/$(basename $@).o $(basename $@).d/' > $@
%.o : %.cpp
	$(CXX) $(CFLAGS) $(INC_FLAG) -c $< -o $@
%.d : %.cpp
	@set -e; $(CXX) $(CFLAGS) $(INC_FLAG) $(CPPFLAGS) -MM $< | sed -e 's/$(basename $@).o/$(basename $@).o $(basename $@).d/' > $@

$(LIB) : $(OBJS)
	$(AR) r $@ $^

clean:
	$(RM) $(OBJS) $(DEPS) $(LIB)

sinclude $(DEPS)
