# Define compilers
#CPP			= g++
#CPPFLAGS	= $(CPPFLAGS) --pedantic-errors --std=c++11 -Wall -DETYPE=int -DSIZE=2

# Define DETYPE and DSIZE
DETYPE = std::string
#std::string
DSIZE = 4

# List of source files
SOURCE_FILES = simpletest.cpp Exthash.hpp
TARGET_FILE = exthashTest

# Targets
.PHONY: all clean

all:
	g++ --pedantic-errors --std=c++11 -DETYPE=$(DETYPE) -DSIZE=$(DSIZE) -Wall -o $(TARGET_FILE) $(SOURCE_FILES)

clean:
	-@rm -f *.o
	-@rm $(TARGET_FILE)
