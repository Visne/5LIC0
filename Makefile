MOTES ?= client server coap-client coap-server price-server hello-world

all: $(MOTES)
clean: $(addprefix clean-, $(MOTES))
distclean: $(addprefix distclean-, $(MOTES))

$(MOTES):
	make -C motes/$@/ TARGET=cooja

clean-%:
	make -C	motes/$*/ TARGET=cooja clean

distclean-%:
	make -C	motes/$*/ TARGET=cooja distclean
