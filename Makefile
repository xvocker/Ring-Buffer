all:
	gcc shmread.c -o read
	gcc shmwrite.c -o write
clean:
	rm read write
