COL_ON = \033[0;32m
COL_OFF = \033[m


%.o : %.cpp ${LIBS}
	echo "${COL_ON}Compiling $< ${COL_OFF}"
	$(CC) $(CXXFLAGS) -c $< -o $@

#Variables
# Compiler to use
CXX=g++
# Flag for release mode
RELEASE=-O2
# Flag for debug mode
DEBUG=-DDEBUG -g -DVAGG_DEBUG
# Compiler flags
CPPFLAGS=-Wall -ansi -Wextra -std=c++0x -g -D DEBUG -D VAGG_DEBUG
# Directory where the source files are
SRC=src
# Directory for the object files
OBJ=obj
# Directory for the final binary
BIN=bin
# Directory for the documentation
DOC=doc
# Directory for the report
REPORT=report
# Directories to create
DIRS=obj recordings bin

# Libraries
# Dynamic libraries to link with
LDFLAGS=-lsndfile -lrt -lasound -lpthread -lportaudio -Wl,-rpath -Wl,/usr/local/lib/ -L/usr/local/lib -I.
# Static libraries to link with
STATIC=

# Rules
$(OBJ)/%.o : $(SRC)/%.cpp $(SRC)/%.h
	@echo "${COL_ON}Compiling $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -c $< -o $@

$(OBJ)/%.o : $(SRC)/%.cpp
	@echo "${COL_ON}Compiling $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS)  $(LDFLAGS) -c $< $(STATIC) -o $@

$(OBJ)/%.o : $(SRC)/%.h
	@echo "${COL_ON}Compiling $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS)  $(LDFLAGS) -c $< $(STATIC) -o $@

$(BIN)/%: $(OBJ)/%.o
	@echo "${COL_ON}Linking $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -o $@

# Targets
all : $(BIN)/read_file $(BIN)/write_file $(BIN)/read_file_buffer

clean :
	@echo "Cleaning $(BIN) & $(OBJ)"
	rm -r $(OBJ)/*
	@echo "Directories emptied."

mrproper:
	@echo "Cleaning $(BIN), $(OBJ) & $(DOC)..."
	rm -r $(BIN)/* $(OBJ)/* $(DOC)/*
	@echo "Project directories are now clean."

# Dependencies
# Format : $(OBJ)/*.o : [$(SRC)/*.hpp]+
$(OBJ)/write_file.o: ${SRC}/write_file.cpp
$(OBJ)/read_file.o: ${SRC}/read_file.cpp
$(OBJ)/read_file_buffer.o: ${SRC}/read_file_buffer.cpp

$(shell   mkdir -p $(DIRS))
