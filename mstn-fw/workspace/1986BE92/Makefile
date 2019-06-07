PROJECT  = 1986BE92_blink

AS       = arm-none-eabi-gcc -x assembler-with-cpp
CC       = arm-none-eabi-gcc
CPP      = arm-none-eabi-g++
LD       = arm-none-eabi-g++
OBJCOPY  = arm-none-eabi-objcopy
SZ       = arm-none-eabi-size
RM       = rm

C_FLAGS += -mcpu=cortex-m3 -mthumb
C_FLAGS += -Og
C_FLAGS += -g3
C_FLAGS += -std=gnu11 -MMD -MP
C_FLAGS += -DMDR1986VE9x=1 
C_FLAGS += -DDEBUG_ON
C_FLAGS += -DHSE_VALUE=8000000
C_FLAGS += -DF_CPU=80000000
C_FLAGS += -DF_OSC=8000000

CPP_FLAGS += -mcpu=cortex-m3 -mthumb 
CPP_FLAGS += -Og
CPP_FLAGS += -g3
CPP_FLAGS += -std=gnu++11 -fabi-version=0 -MMD -MP
CPP_FLAGS += -DMDR1986VE9x=1
CPP_FLAGS += -DDEBUG_ON
CPP_FLAGS += -DHSE_VALUE=8000000
CPP_FLAGS += -DF_CPU=80000000
CPP_FLAGS += -DF_OSC=8000000

PROJECT_DIR = ./src
PROJECT_INCLUDE = ./src

MACS_PATH = ../../toolchain/macs

LD_SCRIPT = $(MACS_PATH)/target/1986ve92/toolchain/gcc/1986ve92.ld

INCLUDE_PATHS += $(PROJECT_INCLUDE)
SOURCES_DIR += $(PROJECT_DIR)

# OS
INCLUDE_PATHS += $(MACS_PATH)/include
INCLUDE_PATHS += $(MACS_PATH)/src
INCLUDE_PATHS += $(MACS_PATH)/src/application
INCLUDE_PATHS += $(MACS_PATH)/src/clock
INCLUDE_PATHS += $(MACS_PATH)/src/ipc
INCLUDE_PATHS += $(MACS_PATH)/src/lib
INCLUDE_PATHS += $(MACS_PATH)/src/log
INCLUDE_PATHS += $(MACS_PATH)/src/memory
INCLUDE_PATHS += $(MACS_PATH)/src/profiler
INCLUDE_PATHS += $(MACS_PATH)/src/sync

#new targets
INCLUDE_PATHS += $(MACS_PATH)/target/1986ve92/toolchain/gcc/
INCLUDE_PATHS += $(MACS_PATH)/target/1986ve92/toolchain/gcc/newlib
INCLUDE_PATHS += $(MACS_PATH)/target/1986ve92/src/
INCLUDE_PATHS += $(MACS_PATH)/target/

INCLUDE_PATHS += ./src/third-party/cmsis/
INCLUDE_PATHS += ./src/third-party/SPL/
 
# OS
SOURCES_DIR += $(MACS_PATH)/include
SOURCES_DIR += $(MACS_PATH)/src
SOURCES_DIR += $(MACS_PATH)/src/application
SOURCES_DIR += $(MACS_PATH)/src/clock
SOURCES_DIR += $(MACS_PATH)/src/ipc
SOURCES_DIR += $(MACS_PATH)/src/lib
SOURCES_DIR += $(MACS_PATH)/src/log
SOURCES_DIR += $(MACS_PATH)/src/memory
SOURCES_DIR += $(MACS_PATH)/src/profiler
SOURCES_DIR += $(MACS_PATH)/src/sync

#new targets
SOURCES_DIR += $(MACS_PATH)/target/1986ve92/toolchain/gcc/
SOURCES_DIR += $(MACS_PATH)/target/1986ve92/toolchain/gcc/newlib
SOURCES_DIR += $(MACS_PATH)/target/1986ve92/toolchain/
SOURCES_DIR += $(MACS_PATH)/target/1986ve92/src/
SOURCES_DIR += $(MACS_PATH)/target/

SOURCES_DIR += ./src/third-party/cmsis/
SOURCES_DIR += ./src/third-party/SPL/

SOURCES_CPP_WILDCARDS += $(addsuffix /*.cpp,$(SOURCES_DIR))
SOURCES_C_WILDCARDS += $(addsuffix /*.c,$(SOURCES_DIR))
SOURCES_S_WILDCARDS += $(addsuffix /*.S,$(SOURCES_DIR))

OBJDIR = obj
OBJS += $(notdir $(patsubst %.cpp, %.o, $(wildcard $(SOURCES_CPP_WILDCARDS))))
OBJS += $(notdir $(patsubst %.c, %.o, +$(wildcard $(SOURCES_C_WILDCARDS))))
OBJS += $(notdir $(patsubst %.S, %.o, $(wildcard $(SOURCES_S_WILDCARDS))))
OBJECTS = $(addprefix $(OBJDIR)/, $(OBJS))

PROJECT_DEBUGS = $(patsubst %.o,%.d,$(OBJECTS))

LD_FLAGS = -nostartfiles -Wl,-Map,"$(PROJECT).map" --specs=nano.specs

VPATH = $(SOURCES_DIR)

all: $(PROJECT).elf size toobj

clean:
	$(RM) -rf $(PROJECT).elf $(PROJECT).map $(OBJECTS) $(PROJECT_DEBUGS) *.bin $(PROJECT).hex $(PROJECT).bin

$(OBJDIR)/%.o: %.S
	$(AS) $(C_FLAGS) $(AS_FLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.c
	$(CC) $(C_FLAGS) $(addprefix -I, $(INCLUDE_PATHS)) $(GCC_INCLUDES) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	$(CPP) $(CPP_FLAGS) $(addprefix -I, $(INCLUDE_PATHS)) $(GCC_INCLUDES) -c -o $@ $<

$(PROJECT).elf: $(OBJECTS)
	$(LD) $(CPP_FLAGS) -T$(LD_SCRIPT) $(LD_FLAGS) -o "$@" $^

size:
	$(SZ) $(PROJECT).elf

toobj:
	$(OBJCOPY) -O binary $(PROJECT).elf  $(PROJECT).bin
	$(OBJCOPY) -O ihex $(PROJECT).elf  $(PROJECT).hex