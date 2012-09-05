PREFIX?=/usr/local

CFLAGS=-g -Wall -I${PREFIX}/include
LIBS=-L${PREFIX}/lib -lbsdconv -lopencc

all: map callback

map: OPENCC.txt
	bsdconv_mktable OPENCC.txt OPENCC

callback: OPENCC.c
	$(CC) ${CFLAGS} -fPIC -shared -o OPENCC.so OPENCC.c ${LIBS}

clean:
	rm -rf OPENCC OPENCC.so

install:
	install -m 444 OPENCC ${PREFIX}/share/bsdconv/inter/OPENCC
	install -m 444 OPENCC.so ${PREFIX}/share/bsdconv/inter/OPENCC.so
