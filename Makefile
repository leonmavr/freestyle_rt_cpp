#================================================= Makefile =================================================

# Compiles the project assuming it adheres to the following structure:
#
# <root directory>
# |
# +---- <SRC_DIR> ---+---- subdir1/*.cpp, *.hpp
# |                  +---- subdir2/*.cpp, *.hpp                       
# |                  +---- main.cpp
#
# All headers are now co-located with their .cpp files inside src/.

SHELL    := /bin/bash
CXX      := g++
EXEC     := demo 

SRC_DIR  := src
OBJ_DIR  := build

# include all subdirectories in src/ (for headers)
INCDIRS  := $(shell find $(SRC_DIR) -type d 2>/dev/null || true)
INCLUDES := $(patsubst %,-I%,$(INCDIRS))

CXXFLAGS := $(INCLUDES) $(SDL_CFLAGS) -O3 -std=c++17 -Wall -Wextra -MMD -MP
LDFLAGS  := -lm
LDLIBS   :=

# all cpp files under src/
SRCS := $(shell find $(SRC_DIR) -type f -name '*.cpp' -print)

# map src/... to build/src/...
SRC_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/$(SRC_DIR)/%.o,$(SRCS))
OBJECTS  := $(SRC_OBJS)

# dependency files
DEPS := $(OBJECTS:.o=.d)

.PHONY: all clean rebuild

all: $(EXEC)
	@echo -e "\n======== Final executable at: ./$(EXEC) ========"

# link everything together 
$(EXEC): $(OBJECTS)
	@echo -e "\n======== Linking $@ ========"
	$(CXX) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# compile rules
$(OBJ_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo -e "\n======== Compiling $< -> $@ ========"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# include dependency info
-include $(DEPS)

clean:
	@echo -e "\n======== Cleaning build artifacts ========"
	@rm -rf $(OBJ_DIR) $(EXEC)

rebuild: clean all

