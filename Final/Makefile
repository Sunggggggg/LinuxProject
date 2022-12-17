obj-m += gpio_driver.o
obj-m += fnd_driver.o
obj-m += motor_driver.o

KDIR 	= ~/working/kernel
RESULT 	= main
SRC 	= $(RESULT).c
CCC 	= gcc

all:
	make -C $(KDIR) M=$(PWD) modules
	$(CCC) -o $(RESULT) $(SRC)

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f $(RESULT)
