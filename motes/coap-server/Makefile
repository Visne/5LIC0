CONTIKI_PROJECT = coap-server
MAKE_MAC = MAKE_MAC_TSCH
PLATFORMS_EXCLUDE = sky z1
MODULES += os/net/app-layer/coap
MODULES_REL += ./resources ../shared
CFLAGS += -Wno-format-overflow

all: $(CONTIKI_PROJECT)

include ../../Makefile.path

# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap
include $(CONTIKI)/Makefile.include
