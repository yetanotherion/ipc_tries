all: multi_test erase_sem

clean:
	rm -f *.o erase_sem multi_test

multi_test: multi_test.o utils.o
	gcc -Wall -g -pedantic multi_test.o utils.o -lc -o multi_test

erase_sem: erase_sem.o utils.o
	gcc -Wall -g -pedantic erase_sem.o utils.o -o erase_sem

%.o: %.c
	gcc -Wall -g -pedantic -c $<
