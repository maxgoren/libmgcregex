libregex:
	gcc -c -fPIC src/rb.c -o rb.o
	gcc -c -fPIC src/set.c -o set.o
	gcc -c -fPIC src/stack.c -o stack.o
	gcc -c -fPIC src/nfa.c -o nfa.o
	gcc -c -fPIC src/parseregex.c -o parse.o
	gcc -c -fPIC src/pattern_match.c -o match.o
	gcc -c -fPIC src/regex.c -o regex.o
	gcc -shared -o libmgcregex.so rb.o nfa.o parse.o match.o set.o regex.o stack.o

install:
	mv libmgcregex.so /usr/local/lib
	mkdir /usr/include/mgc/regex
	cp src/*.h /usr/include/mgc/regex

clean:
	rm *.o