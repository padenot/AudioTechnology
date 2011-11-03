CC=g++
CXXFLAGS=-std=c++0x -Wall -Wextra -g -DVAGG_DEBUG
LIBS=-lsndfile

COL_ON = \033[0;32m
COL_OFF = \033[m


%.o : %.c
	@echo "${COL_ON}Compiling $< ${COL_OFF}"
	@$(CC) $(CXXFLAGS) -c $< -o $@

all : read_file

doc :
	doxygen

clean :
	rm *.o
	rm read_file

read_file : read_file.o
	$(CXX) $(LIBS) $(CXXFLAGS) $+ -o $@
