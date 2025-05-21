<$objtype/mkfile>

TARG=gammera
OFILES=src/main.$O src/fetcher.$O

$TARG: $OFILES
$LD -o $target $OFILES

src/main.$O: src/main.c
$CC -c src/main.c

src/fetcher.$O: src/fetcher.c
$CC -c src/fetcher.c

clean:V:
rm -f src/*.[$OS] $TARG
