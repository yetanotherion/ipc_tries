MULTI_TEST_NAME=multi_test

all: multi_test erase_sem

clean:
	rm -f *.o erase_sem $(MULTI_TEST_NAME)

multi_test: multi_test.o utils.o
	gcc -Wall -g -pedantic multi_test.o utils.o -lc -lrt -lpthread -o $(MULTI_TEST_NAME)

erase_sem: erase_sem.o utils.o
	gcc -Wall -g -pedantic erase_sem.o utils.o -lrt -lpthread -o erase_sem

%.o: %.c
	gcc -Wall -g -pedantic -c $<
