all: Makefile.qmake
	make -f Makefile.qmake
	./pong

Makefile.qmake: Pong.pro
	qmake Pong.pro -o Makefile.qmake

clean:
	make -f Makefile.qmake distclean
