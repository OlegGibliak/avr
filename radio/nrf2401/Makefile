DEVICE  = atmega328p
COMPILE = avr-gcc -mmcu=$(DEVICE) -std=c99 -Wall

all: clean
	mkdir bin
	$(COMPILE) -c nrf2401.c -o bin/nrf2401.o
	$(COMPILE) -c radio.c -o bin/radio.o
	mkdir bin/static
	avr-ar rcs bin/static/radio.a bin/nrf2401.o bin/radio.o

clean:
	@rm -rf bin
