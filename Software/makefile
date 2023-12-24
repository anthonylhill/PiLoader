
# Vim note : insert tabs in insert mode by pressing <Ctrl> + v and then <Tab>

all: 1802load 1802scan gpio_set gpio_get 6digitDisplay qblink idle bbserial serial_test 

clean:
	rm -f *.lst
	touch *.c *.asm *.h	

# =====  C Programs for Pi Loader =====

1802scan:  1802scan.c loader_gpio.h
	cc -Wall -o 1802scan 1802scan.c -lpigpio

1802load:  1802load.c loader_gpio.h
	cc -Wall -o 1802load 1802load.c -lpigpio

gpio_set:  gpio_set.c loader_gpio.h
	cc -Wall -o gpio_set gpio_set.c

gpio_get:  gpio_get.c loader_gpio.h
	cc -Wall -o gpio_get gpio_get.c

# ====  1802 Assembler =======

6digitDisplay:  6digitDisplay.asm
	./a18/a18 6digitDisplay.asm -L 6digitDisplay.lst -b 6digitDisplay

qblink:  qblink.asm
	./a18/a18 qblink.asm -L qblink.lst -b qblink

idle:  idle.asm
	./a18/a18 idle.asm -L idle.lst -b idle

bbserial:  bit_bang_serial.asm
	./a18/a18 bit_bang_serial.asm -L bit_bang_serial.lst -b bbserial

serial_test:  serial_test.asm
	./a18/a18 serial_test.asm -L serial_test.lst -b serial_test

