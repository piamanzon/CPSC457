all: outputFIFO outputROBIN

outputFIFO: fifo.cpp
	g++ fifo.cpp -pthread -o outputFIFO

outputROBIN: robin.cpp
	g++ robin.cpp -pthread -o outputROBIN
