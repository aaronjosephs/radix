all:
	clang++ -std=c++11 radix.cpp -o radix  -g -Wall -pedantic -lpthread
clean:
	rm -rf radix
