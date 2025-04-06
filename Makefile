CXX = g++
SRC_DIR = src
INC_DIR = src
TEST_DIR = test
DEMO_DIR = demos

# Add appropriate C++ flags
CXXFLAGS = -O3 -I$(INC_DIR) -Wall -std=c++11
LDFLAGS = -lm

# Get all source files
SRC = $(wildcard $(SRC_DIR)/*.cpp wildcard $(SRC_DIR)/*.hpp)
TEST_SRC = $(wildcard $(TEST_DIR)/*.cpp)
DEMO_SRC = $(wildcard $(DEMO_DIR)/*.cpp)

# Create object file lists
SRC_OBJ = $(SRC:.cpp=.o)
TEST_OBJ = $(TEST_SRC:.cpp=.o)
DEMO_OBJ = $(DEMO_SRC:.cpp=.o)

# select plotter; PLOTTER=PPM to plot as ppm frames
ifeq ($(PLOTTER), PPM)
    CXXFLAGS += -DUSE_PPM
endif

# Set up different target paths based on make goal
ifeq ($(MAKECMDGOALS), test)
    TARGETS = $(patsubst $(TEST_DIR)/%.cpp, $(TEST_DIR)/%, $(TEST_SRC))
else
    TARGETS = $(patsubst $(DEMO_DIR)/%.cpp, $(DEMO_DIR)/%, $(DEMO_SRC))
endif

# Default target
all: $(TARGETS)

# Test target
test: $(TARGETS)

# Rule for building demo executables
$(DEMO_DIR)/%: $(DEMO_DIR)/%.o $(SRC_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Rule for building test executables
$(TEST_DIR)/%: $(TEST_DIR)/%.o $(SRC_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean all test
clean:
	rm -f $(SRC_DIR)/*.o $(TEST_DIR)/*.o $(DEMO_DIR)/*.o $(TARGETS)
