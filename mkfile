<$PLAN9/src/mkhdr

TARG=gammera
OFILES=main.$O fetch.$O parser.$O serve9p.$O


$TARG: $OFILES
	$LD -o $target $OFILES

main.$O: src/main.c src/fetch.h src/parser.h src/serve9p.h
	$CC -c -o $target src/main.c

fetch.$O: src/fetch.c src/fetch.h
	$CC -c -o $target src/fetch.c

parser.$O: src/parser.c src/parser.h
	$CC -c -o $target src/parser.c

serve9p.$O: src/serve9p.c src/serve9p.h src/fetch.h src/parser.h
        $CC -c -o $target src/serve9p.c

clean:V:
	rm -f *.[$OS] $TARG
