all:
	make -C consola
	make -C kernel
	make -C cpu
	make -C memoria

clean:
	make clean -C consola
	make clean -C kernel
	make clean -C cpu
	make clean -C memoria