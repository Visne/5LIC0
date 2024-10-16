PROJECT_SOURCEFILES += ../../libraries/CAN-sim/VirtualCANBus.cpp ../../libraries/CAN-sim/VirtualCANBusWrapper.cpp ../../libraries/CAN-sim/TagNode.cpp
TARGET_LIBFILES += -lstdc++
CONTIKI_PROJECT = coap-client
MAKE_MAC = MAKE_MAC_TSCH
PLATFORMS_EXCLUDE = sky z1
MODULES += os/net/app-layer/coap
MODULES_REL += ../shared

all: $(CONTIKI_PROJECT)

include ../../Makefile.path
include $(CONTIKI)/Makefile.include
