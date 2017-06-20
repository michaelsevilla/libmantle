all: test

test: test.cc libmantle.a
	g++ -Wall test.cc -o test -I. -L. -lmantle

Mantle.o: Mantle.cc
	g++ -Wall -c Mantle.cc

libmantle.a: Mantle.o
	ar rc libmantle.a Mantle.o

libs: libmantle.a

clean:
	rm -f test *.o *.a *.gch
