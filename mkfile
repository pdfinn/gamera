<$PLAN9/src/mkhdr


# name of the resulting executable
TARG=gamera

# object files for the build
OFILES=main.$O fetcher.$O parser.$O serve9p.$O tabs.$O fs.$O
# serve9p.$O provides the consolidated 9P interface

$TARG: $OFILES
	$LD -o $target $OFILES

main.$O: src/main.c src/fetcher.h src/parser.h src/serve9p.h src/tabs.h
        $CC -c -o $target src/main.c

fetcher.$O: src/fetcher.c src/fetcher.h
        $CC -c -o $target src/fetcher.c

parser.$O: src/parser.c src/parser.h
	$CC -c -o $target src/parser.c

serve9p.$O: src/serve9p.c src/serve9p.h src/fetcher.h src/parser.h
        $CC -c -o $target src/serve9p.c

tabs.$O: src/tabs.c src/tabs.h src/fetcher.h src/parser.h
        $CC -c -o $target src/tabs.c

fs.$O: src/fs.c src/fs.h
        $CC -c -o $target src/fs.c

clean:V:
	rm -f *.[$OS] $TARG
