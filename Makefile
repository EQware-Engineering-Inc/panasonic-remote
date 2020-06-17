CC := avr-gcc
AVRDUDE := avrdude
OBJCOPY := avr-objcopy

CPU := atmega328p
F_CPU := 16000000
PROGRAMMER ?= arduino

CFLAGS=-Os -std=c99 -DF_CPU=$(F_CPU) -mmcu=$(CPU)

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
	    $(AVRDUDE) -c $(PROGRAMMER) -p $(CPU) -U flash:w:$(HEX_TARGET) -B250; \
	else \
	    $(AVRDUDE) -c $(PROGRAMMER) -p $(CPU) -P $(DEV) -U flash:w:$(HEX_TARGET) -B250; \
	fi

fuse:
	if [ "$(PROGRAMMER)" == arduino ] && [ -z "$(DEV)" ]; then \
	    echo "Must set DEV" >&2; \
	    exit 1; \
	fi
	if [ -z "$(DEV)" ]; then \
	    $(AVRDUDE) -c $(PROGRAMMER) -p $(CPU) -U lfuse:w:0x62:m -B250;
	else \
	    $(AVRDUDE) -c $(PROGRAMMER) -p $(CPU) -P $(DEV) -U lfuse:w:0x62:m -B250; \
	fi

clean:
	rm -rf $(OBJ) $(TARGET) $(TARGET_ELF) $(TARGET_HEX)

.PHONY: all flash clean
