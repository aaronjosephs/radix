all:
	clang++ -std=c++11 radix.cpp -o radix -O4 
clean:
	rm -rf radix
