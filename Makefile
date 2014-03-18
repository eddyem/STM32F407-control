# output filename
BIN=testproject

# Headers that don't have c-files with same name
#  but project depends on them
INDEPENDENT_HEADERS=stm32f4xx_conf.h usbd_conf.h

# paths
STLIBDIR=stm32_lib
CMSISDIR=cmsis
OBJDIR=mk
USB_LIBDIR=usb_lib
LWIP_LIBDIR=lwip_v1.3.2/src
LWIP_PORT_DIR=lwip_v1.3.2/port/STM32F4x7

# gcc paths
TOOLS_PATH=/usr/arm-none-eabi
TOOLS_PREFIX=arm-none-eabi-

# flags for objcopy & objdump
CPFLAGS=-Obinary --strip-unneeded
ODFLAGS=-S

# common flags for as, gcc & ld
CLFLAGS=-mcpu=cortex-m4 -mthumb -mthumb-interwork -mlittle-endian

# gcc flags
CFLAGS=$(CLFLAGS) -c -mapcs-frame -Os -DARM_SOURCE -Wall -Werror
CFLAGS+=-mfpu=fpv4-sp-d16 -fsingle-precision-constant
CFLAGS+=-I. -I$(CMSISDIR) -I$(STLIBDIR) -I$(USB_LIBDIR)
CFLAGS+=-I$(LWIP_LIBDIR)/include -I$(LWIP_PORT_DIR) -I$(LWIP_PORT_DIR)/Standalone -I$(LWIP_LIBDIR)/include/ipv4
CFLAGS+=-DSTM32F40XX -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=\(\(uint32_t\)12000000\)
CFLAGS+=-DUSBPA -DUSBPB -DSEPARATE_USB_BUF
CFLAGS+=-DUSE_ETH
CFLAGS+=-DUSEAD7794
CFLAGS+=-DEBUG

# linker flags
LDFLAGS=$(CLFLAGS) -static -Wl,--start-group
LDFLAGS+=-Wl,--end-group -Xlinker -Map -Xlinker $(BIN).map -Xlinker
LDFLAGS+=-T ./cmsis/stm32f4xx_flash.ld -o $(BIN)
LDFLAGS+=-Wl,--gc-sections -Wl,-s

# assembler flags
ASFLAGS=$(CLFLAGS) -I./cmsis -I./stm32_lib

# define names
CC=$(TOOLS_PREFIX)gcc
AS=$(TOOLS_PREFIX)as
SIZE=$(TOOLS_PREFIX)size
CP=$(TOOLS_PREFIX)objcopy
OD=$(TOOLS_PREFIX)objdump

# These are main project sources - in current directory
SRC=$(wildcard *.c)

# SPL sources
STM32_LIBSRC=$(wildcard $(STLIBDIR)/*.c)
STM32_LIBSRC+=$(wildcard $(CMSISDIR)/*.c)
ASSRC=$(wildcard $(CMSISDIR)/*.s)
ASOBJ=$(addprefix $(OBJDIR)/, $(ASSRC:$(CMSISDIR)/%.s=%.o))

# USB library sources
USB_SRC=$(wildcard $(USB_LIBDIR)/*.c)

# LWIP library sources
LWIP_API_SRC=$(wildcard $(LWIP_LIBDIR)/api/*.c)
LWIP_CORE_SRC=$(wildcard $(LWIP_LIBDIR)/core/*.c)
LWIP_IPV4_SRC=$(wildcard $(LWIP_LIBDIR)/core/ipv4/*.c)
LWIP_NETIF_SRC=$(wildcard $(LWIP_LIBDIR)/netif/*.c)
LWIP_PORT_SRC=$(wildcard $(LWIP_PORT_DIR)/Standalone/*.c)
LWIP_SRC=$(LWIP_API_SRC) $(LWIP_CORE_SRC) $(LWIP_IPV4_SRC) $(LWIP_NETIF_SRC) $(LWIP_PORT_SRC)

# ALLTOGETHER
ALL_SRC=$(SRC) $(STM32_LIBSRC) $(USB_SRC) $(LWIP_SRC)
ALL_OBJ=$(patsubst %.c, $(OBJDIR)/%.o, $(ALL_SRC))

# VPATH for files autosearching
VPATH=$(sort $(dir $(ALL_SRC)))

all: $(BIN) Makefile

# cc
$(OBJDIR)/%.o : %.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

# as
$(OBJDIR)/%.o: $(CMSISDIR)/%.s Makefile
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

# headers
$(SRC) $(USB_SRC) : %.c : %.h $(INDEPENDENT_HEADERS)
	@touch $@

%.h: ;

$(BIN): $(ALL_OBJ) $(ASOBJ) Makefile
	$(CC) $(ALL_OBJ) $(ASOBJ) $(LDFLAGS)
	$(CP) $(CPFLAGS) $(BIN) $(BIN).bin
	$(OD) $(ODFLAGS) $(BIN) > $(BIN).list
	$(SIZE) -B $(BIN)

clean:
	rm -f $(BIN).map $(BIN) $(BIN).list
	rm -rf $(OBJDIR)

load: $(BIN)
	st-flash write $(BIN).bin 0x08000000

.PHONY: clean load
