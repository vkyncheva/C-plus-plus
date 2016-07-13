ROOTINC = $(shell root-config --cflags) 
ROOTLIBS = $(shell root-config --glibs)

all:
	g++ NoiseVsHV.cpp -o NoiseVsHV.o $(ROOTLIBS) $(ROOTINC)

clean:
	find ./ -maxdepth 10 -type f -iname '*.o' -delete;
	find ./ -maxdepth 10 -type f -iname '*~' -delete
