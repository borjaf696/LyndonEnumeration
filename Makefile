# Makefile
SDSLFLAGS = -L ~/lib -lsdsl -ldivsufsort -ldivsufsort64
CXXFLAGS = -std=c++0x -O3 -fpermissive
# Main obj
main_obj := ${patsubst %.cpp,%.o,${wildcard *.cpp}}
%.o: %.cpp %.h
	${CXX} -c ${CXXFLAGS} $< -o $@
mCodeRelease := lyndon.o
all: ${main_obj}
	${CXX} ${mCodeRelease} -o lyndon.out ${SDSLFLAGS} 
clean:
	-rm ${main_obj}
