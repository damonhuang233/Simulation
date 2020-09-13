all:
	g++ -o prog prog.cpp -O3 -lm -fopenmp

file:
	g++ -o prog prog.cpp -O3 -lm -fopenmp
	./prog > result.txt

clean:
	rm -f ./prog result.txt
