FLAGS_version = -std=c++23
FLAGS_warn    = -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic
FLAGS_all     = $(FLAGS_version) $(FLAGS_warn)

# defining SDL library & adding sdl flags
SDL_DIR    = ../third-party/SDL2
FLAGS_all += -I$(SDL_DIR)/include
SRCS       := $(shell find Agents core Interfaces tools Worlds -name '*.cpp')
SDL_LIBS   = -L$(SDL_DIR)/lib -lSDL2 -lSDL2_image

FLAGS_debug   = $(FLAGS_all) -g                     # Setup for running a debugger
FLAGS_opt     = $(FLAGS_all) -O3 -DNDEBUG           # Fastest possible executable
FLAGS_quick   = $(FLAGS_all) -DNDEBUG               # Fastest possible compilation
FLAGS_grumpy  = $(FLAGS_all) -Wconversion -Weffc++  # Some extra picky warnings

CXX = c++

# Find all headers
HEADERS := $(shell find Agents core Interfaces tools Worlds -name '*.hpp')

# List of all available executables
EXECUTABLES = simple

# changed from only simple to all
all: $(EXECUTABLES)
default: all
debug: debug-simple
opt: opt-simple
grumpy: grumpy-simple

.PHONY: default clean debug-% opt-% quick-% $(EXECUTABLES)

# Generic rule to build default version of any executable (added all cpp files & sdl)
$(EXECUTABLES): %: %_main.cpp $(SRCS) $(HEADERS)
	$(CXX) $@_main.cpp $(SRCS) $(FLAGS_all) $(SDL_LIBS) -o $@

# Generic rule to build debug version of any executable
debug-%:
	$(CXX) $*_main.cpp $(FLAGS_debug) -o $*

# Generic rule to build opt version of any executable
opt-%:
	$(CXX) $*_main.cpp $(FLAGS_opt) -o $*

# Generic rule to build quick version of any executable
quick-%: 
	$(CXX) $*_main.cpp $(FLAGS_quick) -o $*

# Generic rule to build quick version of any executable
grumpy-%: 
	$(CXX) $*_main.cpp $(FLAGS_grumpy) -o $*

FILES_backup	= *~ *.dSYM
FILES_generated = *.o $(EXECUTABLES)

clean:
	rm -rf $(FILES_backup) $(FILES_generated)

