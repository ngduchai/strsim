
INC = -Iinclude
LIB = -lpthread

SRC = src
OBJ = obj
VISUAL_DATA = visual/data
VISUAL_FIGS = visual/figs
TESTS = tests
INCLUDE = include
BIN = bin
DEST = /usr

CC = g++
STD = -std=c++11
DEBUG = -g

DFLAGS = #-DDEBUG_ON
CFLAGS = -Wall -c -O2 $(DEBUG) $(STD) $(DFLAGS)
LFLAGS = -Wall -O2 $(DEBUG) $(STD) $(DFLAGS)
SFLAGS = -Wall -shared -fpic -O2 $(DEBUG) $(STD) $(DFLAGS)

ROOT = .

vpath %.cpp $(SRC)
vpath %.h $(INCLUDE)

MAKE = $(CC) $(INC)

HEADER = $(wildcard $(INCLUDE)/*.h)

# Object files needed by modules
TEST_RAND = tests/rand.cpp $(addprefix $(OBJ)/, code.o store.o)
TEST_CODE = tests/code.cpp $(addprefix $(OBJ)/, code.o)
SIMPLE_SIM = $(addprefix $(OBJ)/, code.o store.o simplesim.o)

all: prepare

prepare:
	mkdir -p $(BIN) $(OBJ) $(VISUAL_DATA) $(VISUAL_FIGS)

test: testrand

# Test random generators
testrand: $(TEST_RAND)
	$(MAKE) $(LFLAGS) $(TEST_RAND) -o $(ROOT)/$(TESTS)/test_rand $(LIB)
	$(ROOT)/$(TESTS)/test_rand

# Test random generators
testcode: $(TEST_CODE)
	$(MAKE) $(LFLAGS) $(TEST_CODE) -o $(ROOT)/$(TESTS)/test_code $(LIB)
	$(ROOT)/$(TESTS)/test_code

# Test random generators
simplesim: $(SIMPLE_SIM)
	$(MAKE) $(LFLAGS) $(SIMPLE_SIM) -o $(ROOT)/$(BIN)/simplesim $(LIB)



# Create objects from sources
$(OBJ)/%.o: %.cpp ${HEADER}
	$(MAKE) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ)/* $(TESTS)/test_* $(BIN)/*


	
