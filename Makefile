CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

library: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o library

run: library
	./library

clean:
	rm -f library books.txt members.txt
