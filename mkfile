<$PLAN9/src/mkhdr

OFILES=main.$O fetcher.$O parser.$O serve9p.$O


$TARG: $OFILES
	$LD -o $target $OFILES

main.$O: src/main.c src/fetcher.h src/parser.h src/serve9p.h
        $CC -c -o $target src/main.c

fetcher.$O: src/fetcher.c src/fetcher.h
        $CC -c -o $target src/fetcher.c

parser.$O: src/parser.c src/parser.h
	$CC -c -o $target src/parser.c

serve9p.$O: src/serve9p.c src/serve9p.h src/fetcher.h src/parser.h
        $CC -c -o $target src/serve9p.c

clean:V:
	rm -f *.[$OS] $TARG
