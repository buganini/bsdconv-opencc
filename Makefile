PREFIX?=/usr/local

CFLAGS=-g -Wall -I${PREFIX}/include
LIBS=-L${PREFIX}/lib -lbsdconv -lopencc

all: map callback

CONFIGS=
CONFIGS+=MIX2ZHS
CONFIGS+=MIX2ZHT
CONFIGS+=ZHS2ZHT
CONFIGS+=ZHS2ZHTW_P
CONFIGS+=ZHS2ZHTW_V
CONFIGS+=ZHS2ZHTW_VP
CONFIGS+=ZHT2ZHS
CONFIGS+=ZHT2ZHTW_P
CONFIGS+=ZHT2ZHTW_V
CONFIGS+=ZHT2ZHTW_VP
CONFIGS+=ZHTW2ZHCN_S
CONFIGS+=ZHTW2ZHCN_T
CONFIGS+=ZHTW2ZHS
CONFIGS+=ZHTW2ZHT

map: OPENCC.txt
	bsdconv-mktable OPENCC.txt OPENCC

callback: OPENCC.c
	for item in ${CONFIGS} ; do \
		cfg=`echo $${item} | tr [A-Z] [a-z]` ; \
		$(CC) ${CFLAGS} -DBSDCONV_OPENCC_CONVERSION='"'$${cfg}'.ini"' -fPIC -shared -o OPENCC-$${item}.so OPENCC.c ${LIBS} ; \
	done
	$(CC) ${CFLAGS} -DBSDCONV_OPENCC_CONVERSION='getenv("BSDCONV_OPENCC")' -fPIC -shared -o OPENCC.so OPENCC.c ${LIBS}

clean:
	rm -rf OPENCC OPENCC.so OPENCC-*

install:
	for item in ${CONFIGS} ; do \
		install -m 444 OPENCC ${PREFIX}/share/bsdconv/inter/OPENCC-$${item} ; \
		install -m 444 OPENCC-$${item}.so ${PREFIX}/share/bsdconv/inter/OPENCC-$${item}.so ; \
	done
	install -m 444 OPENCC ${PREFIX}/share/bsdconv/inter/OPENCC
	install -m 444 OPENCC.so ${PREFIX}/share/bsdconv/inter/OPENCC.so
