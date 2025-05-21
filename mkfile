<$PLAN9/src/mkhdr

TARG=gammera
OFILES=src/main.$O src/fetch.$O src/parser.$O src/render.$O src/fs.$O

$TARG: $OFILES
	 -o $target $OFILES

src/main.$O: src/main.c src/fetch.h src/parser.h src/render.h src/fs.h
	 -c src/main.c

src/fetch.$O: src/fetch.c src/fetch.h
	 -c src/fetch.c

src/parser.$O: src/parser.c src/parser.h
	 -c src/parser.c

src/render.$O: src/render.c src/render.h
	 -c src/render.c

src/fs.$O: src/fs.c src/fs.h
	 -c src/fs.c

clean:V:
	rm -f src/*.[$OS] $TARG

<$PLAN9/src/mkone
