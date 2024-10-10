SOURCES     ?= $(wildcard *.cpp *.c)
OBJECTS     ?= $(SOURCES:.cpp=.o)
OBJECTS     := $(OBJECTS:.c=.o)
DEPS        ?= $(SOURCES:.cpp=.d)
DEPS        := $(DEPS:.c=.d)

test_can: $(OBJECTS)
	@echo Linking $^ into $@
	g++ $^ -o $@

%.o: %.cpp %.c
	@echo Compiling $< into $@
	g++ -I . -std=c++11 -c $< -o $@

%.d: %.cpp %.c
	@echo Generating dependencies for $< into $@
	g++ -I . -MM $< > $@

-include $(DEPS)

clean:
	rm -f *.d *.o