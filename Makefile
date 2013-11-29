all:
	clang++ -std=c++11 radix.cpp -o radix  -O4 -Wall -pedantic
clean:
	rm -rf radix
