CONTIKI_PROJECT = test_virtual_can
BUILD_DIR=$(CURDIR)/build

# Specify the C++ source files
PROJECT_SOURCEFILES += VirtualCANBus.cpp VirtualCANBusWrapper.cpp
OBJECTS = VirtualCANBus.o VirtualCANBusWrapper.o

# Add C++ compilation flags
CXXFLAGS += -std=c++11

# Link the C++ standard library
TARGET_LIBFILES += -lstdc++



all: $(CONTIKI_PROJECT)

test_can: $(BUILD_DIR)/test_can.o $(BUILD_DIR)
			@echo Linking $< into $@
			cc -o $@ $<

$(BUILD_DIR)/test_can.o: test_virtual_can.c $(PROJECT_SOURCEFILES)
			@echo Compiling $< into $@
			cc -c -o $@ $<

$(BUILD_DIR)/VirtualCANBus.o: VirtualCANBus.cpp
	 		@echo Compiling $< into $@
			cc -c -o $@ $<

$(BUILD_DIR)/VirtualCANBusWrapper.o: VirtualCANBusWrapper.cpp
	 		@echo Compiling $< into $@
			cc -c -o $@ $<

clean: 
			rm $(BUILD_DIR)/*.o
# Include Contiki makefiles
# include ../../Makefile.path
# include $(CONTIKI)/Makefile.include
