CXXFLAGS = -DNDEBUG


main.exe: main.cpp
	g++ -pthread -std=c++17 main.cpp -o main.exe


.PHONY: clean

clean:
	rm *.exe *.o
