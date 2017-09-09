
INC = -Iinclude
LIB = -lpthread

SRC = src
OBJ = obj
TESTS = tests
INCLUDE = include
BIN = bin
DEST = /usr

CC = g++
STD = -std=c++11
DEBUG = -g

DFLAGS = 
CFLAGS = -Wall -c -O2 $(DEBUG) $(STD) $(DFLAGS)
LFLAGS = -Wall -O2 $(DEBUG) $(STD) $(DFLAGS)
SFLAGS = -Wall -shared -fpic -O2 $(DEBUG) $(STD) $(DFLAGS)

ROOT = .

vpath %.cpp $(SRC)
vpath %.h $(INCLUDE)

MAKE = $(CC) $(INC)

HEADER = $(wildcard $(INCLUDE)/*.h)

# Object files needed by modules
TEST_RAND = tests/rand.cpp $(addprefix $(OBJ)/, code.o)

all: prepare

prepare:
	mkdir -p $(BIN) $(OBJ)

test: testrand

# Test random generators
testrand: $(TEST_RAND)
	$(MAKE) $(LFLAGS) $(TEST_RAND) -o $(ROOT)/$(TESTS)/test_rand $(LIB)
	$(ROOT)/$(TESTS)/test_rand

# Create objects from sources
$(OBJ)/%.o: %.cpp ${HEADER}
	$(MAKE) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ)/* $(TESTS)/test_* $(BIN)/*


	
