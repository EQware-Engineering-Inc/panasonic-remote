CC := avr-gcc
AVRDUDE := avrdude
OBJCOPY := avr-objcopy

CPU := atmega328p
F_CPU := 1000000
PROGRAMMER ?= usbasp
BITCLOCK ?= -B256kHz

ifdef TUNE_CLOCK
    # Unset bit 7 to output clock to PB0
    FUSE_LOW := 0x22
    CFLAGS += -DTUNE_CLOCK
else
    FUSE_LOW := 0x62
endif

CFLAGS += -Os -std=c99 -DF_CPU=$(F_CPU) -mmcu=$(CPU) -g

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

HEX_TARGET := $(TARGET).hex
ELF_TARGET := $(HEX_TARGET:.hex=.elf)

all: $(HEX_TARGET)

$(HEX_TARGET): $(ELF_TARGET)
	$(OBJCOPY) -O ihex $(ELF_TARGET) $(HEX_TARGET)

$(ELF_TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $+

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

flash: $(HEX_TARGET)
	if [ "$(PROGRAMMER)" == arduino ] && [ -z "$(DEV)" ]; then \
	    echo "Must set DEV" >&2; \
	    exit 1; \
	fi
	if [ -z "$(DEV)" ]; then \
	    $(AVRDUDE) -c $(PROGRAMMER) -p $(CPU) -U flash:w:$(HEX_TARGET) $(BITCLOCK); \
	else \
	    $(AVRDUDE) -c $(PROGRAMMER) -p $(CPU) -P $(DEV) -U flash:w:$(HEX_TARGET) $(BITCLOCK); \
	fi

fuse:
	if [ "$(PROGRAMMER)" == arduino ] && [ -z "$(DEV)" ]; then \
	    echo "Must set DEV" >&2; \
	    exit 1; \
	fi
	if [ -z "$(DEV)" ]; then \
	    $(AVRDUDE) -c $(PROGRAMMER) -p $(CPU) -U lfuse:w:$(FUSE_LOW):m $(BITCLOCK); \
	else \
	    $(AVRDUDE) -c $(PROGRAMMER) -p $(CPU) -P $(DEV) -U lfuse:w:$(FUSE_LOW):m $(BITCLOCK); \
	fi

clean:
	rm -rf $(OBJ) $(TARGET) $(TARGET_ELF) $(TARGET_HEX)

.PHONY: all flash clean
