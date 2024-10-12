SOURCES     := $(wildcard *.cpp *.c)
OBJ_DIR     := obj
DEPS_DIR    := deps
OBJECTS     := $(patsubst %.cpp, %.o, $(SOURCES))
OBJECTS     := $(patsubst %.c, %.o, $(OBJECTS))
DEPS        := $(patsubst %.cpp, %.d, $(SOURCES))
DEPS        := $(patsubst %.c, %.d, $(DEPS))

# Search for source files in the current directory
vpath %.cpp .
vpath %.c .

# Ensure obj/ and deps/ directories are created
$(OBJ_DIR) $(DEPS_DIR):
	@mkdir -p $@

test_can: $(OBJECTS)
	@echo Linking $^ into $@
	g++ $^ -o $@

# Rule for compiling C++ source files
%.o: %.cpp %.c | $(OBJ_DIR)
	@echo Compiling $< into $@
	g++ -I . -std=c++11 -c $< -o $@

# Rule for generating dependencies for C++ files
%.d: %.cpp %.c | $(DEPS_DIR)
	@echo Generating dependencies for $< into $@
	g++ -I . -MM $< > $@

-include $(DEPS)

clean:
	rm -f $(OBJ_DIR)/*.o $(DEPS_DIR)/*.d