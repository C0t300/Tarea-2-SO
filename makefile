TARGET = main
CFLAGS = -Wall -g -lrt

all:
	gcc -c tarea.c $(CFLAGS)
	gcc tarea.c -o $(TARGET) $(CFLAGS)
	make clear

run: all
	./$(TARGET)

clear:
	find . -name '*.o' -type f -delete
