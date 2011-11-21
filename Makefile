COL_ON = \033[0;32m
COL_OFF = \033[m

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
$(OBJ)/%.o : $(SRC)/%.cpp
	@echo "${COL_ON}Compiling $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -c $< -o $@

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

$(BIN)/read_file_buffer: $(OBJ)/read_file_buffer.o $(OBJ)/AudioBuffersQueue.o $(OBJ)/AudioFile.o
	@echo "${COL_ON}Linking $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -o $@

$(BIN)/read_file: $(OBJ)/read_file.o
	@echo "${COL_ON}Linking $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -o $@

$(BIN)/write_file: $(OBJ)/write_file.o
	@echo "${COL_ON}Linking $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -o $@

# Dependencies
# Format : $(OBJ)/*.o : [$(SRC)/*.hpp]+
$(OBJ)/write_file.o: $(SRC)/write_file.cpp
$(OBJ)/read_file.o: $(SRC)/read_file.cpp
$(OBJ)/read_file_buffer.o: $(SRC)/read_file_buffer.cpp
$(OBJ)/AudioBuffersQueue.o: $(SRC)/AudioBuffersQueue.cpp
$(OBJ)/AudioFile.o: $(SRC)/AudioFile.cpp

$(shell mkdir -p $(DIRS))
