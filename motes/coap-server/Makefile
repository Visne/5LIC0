CONTIKI_PROJECT = coap-server
PLATFORMS_EXCLUDE = sky z1
MODULES += os/net/app-layer/coap
MODULES_REL += ./resources
CFLAGS += -Wno-format-overflow

all: $(CONTIKI_PROJECT)

include ../../Makefile.path
include $(CONTIKI)/Makefile.include
