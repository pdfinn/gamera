<$objtype/mkfile>

TARG=gammera
OFILES=src/main.$O src/fetch.$O

$TARG: $OFILES
$LD -o $target $OFILES

src/main.$O: src/main.c src/fetch.h
$CC -c src/main.c

src/fetch.$O: src/fetch.c src/fetch.h
$CC -c src/fetch.c

clean:V:
rm -f src/*.[$OS] $TARG
