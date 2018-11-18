
#include <string.h>
#include <sys/timeb.h>
#include "library.h"
#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include <fstream>


int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Error, usage must be:\n";
        std::cout << "./csv2heapfile <csv_file> <heapfile> <page_size>\n";
        return 1;
    }

    std::ifstream fileCsv;
    fileCsv.open(argv[1]);

    if (!fileCsv)
    {
        std::cout << "Error, could not find file " << argv[1] << "\n";
        return 1;
    }

    bool show_output = true;
    if (argc == 5 && strcmp(argv[4], "--no-output") == 0) {
        show_output = false;
    }

    // initialize page
    int page_size = atoi(argv[3]);
    int record_size = 100 * 10;

    Page page;
    init_fixed_len_page(&page, page_size, record_size);

    // initialize heapfile
    Heapfile *heap = new Heapfile();
    FILE* fileHeap = fopen(argv[2], "w+b");
    if (!fileHeap) {
        std::cout << "Error, could not find file " << argv[2] << "\n";
        return 1;
    }

    init_heapfile(heap, page_size, fileHeap);

    // timing and counting stuff
    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;

    int Tots = 0;
    while (fileCsv) {

        std::string line;
        fileCsv >> line;

        if (line.size() == 0) {
            // ignore empty lines
            continue;
        }

        // remove all commas from the line
        line.erase(std::remove(line.begin(), line.end(), ','), line.end());

        Record *r = new Record;

        // turn 'line' from string to char*, and read the values into r
        fixed_len_read((char*)line.c_str(), record_size, r);

        int slot_index = add_fixed_len_page(&page, r);

        if (slot_index == -1) {  // page is full
            // write page to heap (disk)
            PageID pageID = alloc_page(heap);
            write_page(&page, heap, pageID);

            // allocate empty page
            init_fixed_len_page(&page, page_size, record_size);

            // recalculate slot index
            slot_index = add_fixed_len_page(&page, r);
        }

        write_fixed_len_page(&page, slot_index, r);

        Tots++;
    }

    // write last page to file if it has records
    if (page.used_slots > 0) {
        // write page to heap (disk)
        PageID pageID = alloc_page(heap);
        write_page(&page, heap, pageID);
    }

    ftime(&t);
    long total_run_time = ((t.time * 1000) + t.millitm) - start_time_in_ms;

    fileCsv.close();

    if (show_output) {
        std::cout << "RUN TIME: " << total_run_time << " milliseconds\n";
        std::cout << "TOTAL RECORDS: " << Tots << "\n";
    }
    return 0;
}
