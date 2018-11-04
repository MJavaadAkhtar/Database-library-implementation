# Makefile
CC = g++ 

all: write_fixed_len_pages read_fixed_len_page

library.o: library.cc library.h
	$(CC) -o $@ -c $<

write_fixed_len_pages: write_fixed_len_pages.cc library.o
	$(CC) -o $@ $< library.o

read_fixed_len_page: read_fixed_len_page.cc library.o
	$(CC) -o $@ $< library.o

clean :$
	\rm -fr write_fixed_len_pages read_fixed_len_page *.o