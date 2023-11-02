
# insert tab in insert mode by pressing <Ctrl> + v and then <Tab>

# all: 1802setup serial_tx load_program serial_echo readQ \
#     test18.bin 1802_bit_bang_serial.bin serial_test.bin

all: 1802load 1802scan gpio_set gpio_get mtest2 in_out 6digitDisplay qblink idle bbserial serial_test 


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

in_out:  in_out.asm
	./a18/a18 in_out.asm -L in_out.lst -b in_out

6digitDisplay:  6digitDisplay.asm
	./a18/a18 6digitDisplay.asm -L 6digitDisplay.lst -b 6digitDisplay

qblink:  qblink.asm
	./a18/a18 qblink.asm -L qblink.lst -b qblink

idle:  idle.asm
	./a18/a18 idle.asm -L idle.lst -b idle

bbserial:  1802_bit_bang_serial.asm
	./a18/a18 1802_bit_bang_serial.asm -L 1802_bit_bang_serial.lst -b bbserial

serial_test:  serial_test.asm
	./a18/a18 serial_test.asm -L serial_test.lst -b serial_test

