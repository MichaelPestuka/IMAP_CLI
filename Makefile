.PHONY: all archive

all: src/*.cpp 
	g++ src/*.cpp -o imapcl -g -lssl -lcrypto -Wall -Werror -Wzero-as-null-pointer-constant

run: all
	./imapcl

seznam: all
	./imapcl 77.75.78.99 -a auth_file -o ./

test:
	g++ test.cpp -o test -lssl -lcrypto

archive:
	zip xpestu01.zip *.c *.h Makefile CHANGELOG.md README.md LICENSE
