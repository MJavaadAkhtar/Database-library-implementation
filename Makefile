CC = g++ -std=c++11

library.o: library.cc library.h
	$(CC) -g -Wall -o $@ -c $<

library: library.o

csv2heapfile: csv2heapfile.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

scan: scan.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

insert: insert.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

update: update.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

delete: delete.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

select: select.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

csv2colstore: csv2colstore.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

select2: select2.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

select3: select3.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

write_fixed_len_pages: write_fixed_len_pages.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

read_fixed_len_page: read_fixed_len_page.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

test_heapfile: test_heapfile.cc library.o
	$(CC) -g -Wall -o $@ $< library.o

all: csv2heapfile csv2colstore scan insert update delete select select2 select3 write_fixed_len_pages read_fixed_len_page

clean:
	rm -rf *.o csv2heapfile csv2colstore scan insert update delete select select2 select3 write_fixed_len_pages read_fixed_len_page *.dSYM
