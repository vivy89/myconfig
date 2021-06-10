CROSS_COMPILE :=
GCC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
RM := rm

TARGET := myconfig

MFILE_PATH :=  . \
	./mychar

MINCLUDE_PATH := $(MFILE_PATH) \
				./include

MCDEFINE := -g
MINCLUDE := $(patsubst %, -I%, $(MINCLUDE_PATH))
MCFLAGS := -c $(MINCLUDE) $(MCDEFINE)

MLD_LIB := 	
MLD_FLAGS := $(patsubst %, -l%, $(MLD_LIB))

SRCS := $(wildcard $(addsuffix /*.c, $(MFILE_PATH)))
OBJS := $(patsubst %.c, %.o, $(SRCS))

all:$(TARGET)
	
$(TARGET):$(OBJS)
	$(GCC) $(MLD_FLAGS) -o $@ $^

%.o:%.c
	$(GCC) $(MCFLAGS) -o $@ $^

.PHONY:clean
clean:
	@$(RM) $(TARGET) $(OBJS) -rvf



