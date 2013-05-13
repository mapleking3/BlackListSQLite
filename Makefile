APP		:= exe
CSCRS	:= test.c plate_bw_list.c
CXXSCRS := 
COBJS	:= $(CSCRS:.c=.o)
CXXOBJS := $(CXXSCRS:.cpp=.o)
OBJS	:= $(COBJS) $(CXXOBJS)

LD_FLAG := -lrt -lsqlite3 -pthread -L. 
CFLAGS	:= -g -Wall

LIBS	:= 

%.o:%.c 
	gcc $(CFLAGS) -c $< -o $@
%.o:%.cpp
	g++ $(CFLAGS) -c $< -o $@
$(APP):$(OBJS)
	g++ $(CFLAGS) $(OBJS) -o $@ $(LD_FLAG)  

.PHONY: clean
clean:
	-@rm -rf $(COBJS) $(APP) test.db
#-@rm -rf `ls -Itest.c -Itest.h -IMakefile`

