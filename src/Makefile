all: ADC2014

CC = g++

CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -I/usr/local/include -L/usr/local/lib -lspatialindex -lboost_system -lboost_thread

HDR = adc2014_utility.h	config.h global_func.h global_value.h NN_RST.h

ADC2014: adc2014_main.cpp $(HDR)
	$(CC) $(CXXFLAGS) -o$@ adc2014_main.cpp

clean:
	rm -rf ADC2014
