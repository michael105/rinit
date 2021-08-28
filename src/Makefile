
BINARIES := rinit.run rinit.boot reboot halt init rd.note rd.wait
CONF := minilib.conf





all: $(BINARIES)



%: %.c config.h
	make -f Makefile.minilib BUILD=$@


clean:
	rm $(BINARIES)


