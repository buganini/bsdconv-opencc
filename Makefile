DESTDIR?=
PREFIX?=/usr/local
LOCALBASE?=${PREFIX}

CFLAGS=-Wall -D_BSDCONV_INTERNAL -I${LOCALBASE}/include
LIBS=-L${LOCALBASE}/lib -lbsdconv -lopencc

all: map callback

map: OPENCC.txt
	bsdconv-mktable OPENCC.txt OPENCC

callback: OPENCC.c
	$(CC) ${CFLAGS} -fPIC -shared -o OPENCC.so OPENCC.c ${LIBS}

clean:
	rm -rf OPENCC OPENCC.so OPENCC-*

install:
	install -m 444 OPENCC ${DESTDIR}${PREFIX}/share/bsdconv/inter/OPENCC
	install -m 444 OPENCC.so ${DESTDIR}${PREFIX}/share/bsdconv/inter/OPENCC.so
	install -m 444 OPENCC.man ${DESTDIR}${PREFIX}/share/bsdconv/inter/OPENCC.man
