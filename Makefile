all: 1730sh

run: 1730sh
	./1730sh

1730sh: 1730sh.o Input.o
	g++ -o 1730sh 1730sh.o Input.o

1730sh.o: 1730sh.cpp
	g++ -c -g -Wall -std=c++14 -pedantic-errors 1730sh.cpp

Input.o: Input.cpp
	g++ -c -g -Wall -std=c++14 -pedantic-errors Input.cpp

clean: 
	rm -f *.o
	rm -f *~
	rm -f 1730sh