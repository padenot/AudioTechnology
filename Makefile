COL_ON = \033[0;32m
COL_OFF = \033[m

#Variables
# Compiler to use
CXX=g++-4.5
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
LDFLAGS=-lsndfile -lrt -lasound -lpthread -lportaudio -Wl,-rpath -Wl,/usr/local/lib/ -Wl,-rpath -Wl,../vagg -L/usr/local/lib -I. -Lvagg -lvagg  -lm
# Static libraries to link with
STATIC=

# Rules
$(OBJ)/%.o : $(SRC)/%.cpp
	@echo "${COL_ON}Compiling $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -c $< -o $@

# Targets
all :  $(BIN)/read_file_buffers_refactor $(BIN)/write_file_buffers $(BIN)/write_file_buffers_refactor qt-recorder/recorder qt-player/player
#all : $(BIN)/read_file $(BIN)/write_file $(BIN)/read_file_buffer $(BIN)/ringbuffer_test  $(BIN)/read_file_buffers_refactor

clean :
	@echo "Cleaning $(BIN) & $(OBJ)"
	rm -r $(OBJ)/*
	@echo "Directories emptied."

mrproper:
	@echo "Cleaning $(BIN), $(OBJ) & $(DOC)..."
	rm -r $(BIN)/* $(OBJ)/* $(DOC)/*
	@echo "Project directories are now clean."

qt-recorder/recorder:
	cd qt-recorder && qmake recorder.pro && make

qt-player/player:
	cd qt-player && qmake player.pro && make

#$(BIN)/read_file_buffer: $(OBJ)/read_file_buffer.o $(OBJ)/AudioFile.o
	#@echo "${COL_ON}Linking $< ...${COL_OFF}"
	#$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -o $@

#$(BIN)/read_file: $(OBJ)/read_file.o
	#@echo "${COL_ON}Linking $< ...${COL_OFF}"
	#$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -o $@

$(BIN)/write_file: $(OBJ)/write_file.o
	@echo "${COL_ON}Linking $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -o $@

$(BIN)/ringbuffer_test: $(OBJ)/ringbuffer_test.o
	@echo "${COL_ON}Linking $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -D DEBUG_RINGBUFFER -o $@

$(BIN)/write_file_buffers: $(OBJ)/write_file_buffers.o $(OBJ)/AudioFile.o
	@echo "${COL_ON}Linking $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -D DEBUG_RINGBUFFER -o $@

$(BIN)/read_file_buffers_refactor: $(OBJ)/read_file_buffers_refactor.o $(OBJ)/AudioFile.o $(OBJ)/AudioPlayer.o
	@echo "${COL_ON}Linking $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -D DEBUG_RINGBUFFER -o $@

$(BIN)/write_file_buffers_refactor: $(OBJ)/write_file_buffers_refactor.o $(OBJ)/AudioFile.o $(OBJ)/AudioRecorder.o
	@echo "${COL_ON}Linking $< ...${COL_OFF}"
	$(CXX) $(CPPFLAGS) $+ $(LDFLAGS) $(STATIC) -D DEBUG_RINGBUFFER -o $@

# Dependencies
# Format : $(OBJ)/*.o : [$(SRC)/*.hpp]+
$(OBJ)/write_file.o: $(SRC)/write_file.cpp
$(OBJ)/read_file.o: $(SRC)/read_file.cpp
$(OBJ)/read_file_buffer.o: $(SRC)/read_file_buffer.cpp
$(OBJ)/AudioFile.o: $(SRC)/AudioFile.cpp
$(OBJ)/ringbuffer_test.o: $(SRC)/ringbuffer_test.cpp $(SRC)/RingBuffer.hpp
$(OBJ)/write_file_buffers.o: $(SRC)/write_file_buffers.cpp $(SRC)/AudioFile.cpp
$(OBJ)/read_file_buffers_refactor.o: $(SRC)/write_file_buffers_refactor.cpp $(SRC)/AudioFile.cpp $(SRC)/RingBuffer.hpp
$(OBJ)/AudioPlayer.o: $(SRC)/AudioPlayer.cpp $(SRC)/AudioFile.cpp $(SRC)/RingBuffer.hpp
$(OBJ)/AudioRecorder.o: $(SRC)/AudioRecorder.cpp $(SRC)/AudioFile.cpp $(SRC)/RingBuffer.hpp


