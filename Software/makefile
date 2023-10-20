
# make file for 1802 Membership Card PiZero Loader programs

all: 1802load 1802scan gpio_set gpio_get 1802scan 6digitDisplay idiotX0 qblink idle bbserial serial_test   

# =====  C Programs for Pi Loader =====

1802scan:  1802scan.c loader_utils.h loader_gpio.h
	cc -Wall -o 1802scan 1802scan.c -lpigpio

1802load:  1802load.c loader_utils.h loader_gpio.h
	cc -Wall -o 1802load 1802load.c -lpigpio

gpio_set:  gpio_set.c loader_utils.h loader_gpio.h
	cc -Wall -o gpio_set gpio_set.c

gpio_get:  gpio_get.c loader_utils.h loader_gpio.h
	cc -Wall -o gpio_get gpio_get.c

# ====  1802 Assembler =======

mtest2:  mtest2.asm
	./a18/a18 mtest2.asm -L mtest2.lst -b mtest2

6digitDisplay:  6digitDisplay.asm
	./a18/a18 6digitDisplay.asm -L 6digitDisplay.lst -b 6digitDisplay

idiotX0:  idiotX0.asm
	./a18/a18 idiotX0.asm -L idiotX0.lst -b idiotX0

qblink:  qblink.asm
	./a18/a18 qblink.asm -L qblink.lst -b qblink

idle:  idle.asm
	./a18/a18 idle.asm -L idle.lst -b idle

bbserial:  1802_bit_bang_serial.asm
	./a18/a18 1802_bit_bang_serial.asm -L 1802_bit_bang_serial.lst -b bbserial

serial_test:  serial_test.asm
	./a18/a18 serial_test.asm -L serial_test.lst -b serial_test

