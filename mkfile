<$objtype/mkfile>

TARG=gammera
OFILES=src/main.$O

$TARG: $OFILES
$LD -o $target $OFILES

src/main.$O: src/main.c
$CC -c src/main.c

clean:V:
rm -f src/*.[$OS] $TARG
