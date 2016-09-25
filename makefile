objects = FIR_fixedPoint.o \
		  wavcoef.o
		  
fix: $(objects)
	g++ -o fix $(objects)

FIR_fixedPoint.o : FIR_fixedPoint.cpp wavfix.h
	g++ -std=c++11 -c FIR_fixedPoint.cpp
wavcoef.o: wavcoef.cpp wavfix.h
	g++ -std=c++11 -c wavcoef.cpp

.PHONY: clean
clean: 
	rm fix $(objects)
