<$objtype/mkfile>

TARG=gammera
OFILES=src/main.$O src/fetch.$O src/parser.$O

$TARG: $OFILES
$LD -o $target $OFILES

src/main.$O: src/main.c src/fetch.h src/parser.h
$CC -c src/main.c

src/fetch.$O: src/fetch.c src/fetch.h
$CC -c src/fetch.c

src/parser.$O: src/parser.c src/parser.h
$CC -c src/parser.c

clean:V:
rm -f src/*.[$OS] $TARG
