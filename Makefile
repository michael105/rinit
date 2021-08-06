

all:
	cd src && make

install:
	cd src && make install

withclang:
	cd src && CLANG=1 make

