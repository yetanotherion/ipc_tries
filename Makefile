all: multi_test lib erase_sem

clean:
	rm -f *.o erase_sem multi_test libsimtime.so 

lib: sim_time.o
	gcc -shared -Wl,-soname,libsimtime.so -lc -lrt -lpthread -o libsimtime.so sim_time.o

multi_test: multi_test.o sim_time.o
	gcc -Wall -g -pedantic multi_test.o sim_time.o -lc -lrt -lpthread -o multi_test

erase_sem: erase_sem.o
	gcc -Wall -g -pedantic erase_sem.o -lrt -lpthread -o erase_sem

%.o: %.c
	gcc -Wall -g -pedantic -c -fPIC $<
