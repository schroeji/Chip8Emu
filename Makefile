##
# Chip8Emu
#
# @file
# @version 0.1
# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall -MD -std=c++17

# the build target executable:
TARGET = Chip8Emu

#includes
INC= -lSDL2main -lSDL2 -lSDL2_mixer -I ./

SRC=$(wildcard ./main.cpp ./chip8.cpp)

all: $(TARGET)
all: CFLAGS += -O3
debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INC) -o $(TARGET) $(SRC)

clean:
	$(RM) $(TARGET) $(TARGET_DUMPER)


# end
