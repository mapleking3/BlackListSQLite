TARGET	:= generate

CC		:= gcc

CSRCS	:= generate.c

COBJS	:= $(CSRCS:.c=.o)

INC_FLAGS 	+=
C_FLAGS		+= -g -Wall -W
LD_FLAGS	+= 

all:	$(TARGET)

%.o : %.c
	$(CC) -c $< -o $@ $(C_FLAGS) $(INC_FLAGS)

$(TARGET): $(COBJS)
	$(CC) -o $(TARGET) $(COBJS) $(LD_FLAGS)

.PHONY: clean

clean:
	-@rm -rf $(COBJS) $(TARGET)

