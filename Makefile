obj-m+=ismessage.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) testwrite.c -o testwrite
	$(CC) testread.c -o testread
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm testwrite
	rm testread
