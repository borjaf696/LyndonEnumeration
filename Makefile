# Makefile
SDSLFLAGS = -L ~/lib -lsdsl -ldivsufsort -ldivsufsort64 
OPENMPFLAG = -fopenmp
LDLIBS=-lstdc++fs
CXXFLAGS = -std=c++17 -O3 -fpermissive
# Main obj
main_obj := ${patsubst %.cpp,%.o,${wildcard *.cpp}}
%.o: %.cpp %.h
	${CXX} -Wall ${CXXFLAGS} ${OPENMPFLAG} -c $< -o $@
mCodeRelease := lyndon.o
all: ${main_obj}
	${CXX} -Wall ${mCodeRelease} -o lyndon.out ${SDSLFLAGS} ${LDLIBS} ${OPENMPFLAG}
clean:
	-rm ${main_obj}
