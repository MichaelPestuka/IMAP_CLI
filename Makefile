.PHONY: all archive

all: src/*.cpp 
	g++ *.cpp -o imapcl -g -lssl -lcrypto

run: all
	./imapcl

test:
	g++ test.cpp -o test -lssl -lcrypto

archive:
	zip xpestu01.zip *.c *.h Makefile CHANGELOG.md README.md LICENSE
