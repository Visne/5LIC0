all:
	make -C motes/client/ client.cooja TARGET=cooja
	make -C motes/server/ server.cooja TARGET=cooja

distclean:
	make -C	motes/client/ distclean TARGET=cooja
	make -C	motes/server/ distclean TARGET=cooja
