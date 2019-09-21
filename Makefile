CFLAGS = -Wall -g -pthread
all: part1 part2
part1: part1.o
	gcc $(CFLAGS) part1.o -lm -lpthread -o part1
part1.o: part1.c
	gcc $(CFLAGS) -c part1.c
part2: part2.o
	gcc $(CFLAGS) part2.o -lm -lpthread -o part2
part2.o: part2.c
	gcc $(CFLAGS) -c part2.c
clean:
	rm -f *.o part1
	rm -f *.o part2