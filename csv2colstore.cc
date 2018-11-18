#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include "library.h"

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Error, usage must be:\n";
        std::cout << "./csv2colstore <csv_file> <colstore_name> <page_size>\n";
        return 1;
    }

    bool show_output = true;
    if (argc == 5 && strcmp(argv[4], "--no-output") == 0) {
        show_output = false;
    }

    std::ifstream csv_file;
    csv_file.open(argv[1]);
    if (!csv_file) {
        std::cout << "Error, could not find file " << argv[1] << "\n";
        return 1;
    }

    char *colstore_directory = argv[2];
    if (mkdir(colstore_directory, 0700) != 0) {
        std::cout << "Error, could not create directory " << colstore_directory << "\n";
        return 1;
    }

    int page_size = atoi(argv[3]);

    int record_size = NUM_ATTRIBUTES * ATTRIBUTE_SIZE;

    std::vector<Record> *all_records = new std::vector<Record>();

    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;

    int total_records = 0;
    // read csv into list of records
    while (csv_file) {
        std::string line;
        csv_file >> line;

        if (line.size() == 0) {
            // ignore empty lines
            continue;
        }

        // remove all commas from the line
        line.erase(std::remove(line.begin(), line.end(), ','), line.end());

        Record *r = new Record;

        // turn 'line' from string to char*, and read the values into r
        fixed_len_read((char*)line.c_str(), record_size, r);

        all_records->push_back(*r);

        delete r;

        total_records++;
    }

    csv_file.close();

    // create column file for each attribute
    for (int i = 0; i < NUM_ATTRIBUTES; ++i) {
        // column file should be /{directory_path}/{column_index}
        std::ostringstream path_stream;
        path_stream << colstore_directory << "/" << i;
        std::string tmp = path_stream.str();
        const char* path = tmp.c_str();

        FILE *column_heapfile = fopen(path, "w+");
        if (!column_heapfile) {
            std::cout << "Error, could not open file " << path << " for column " << i << "\n";
            return 1;
        }

        Heapfile *heap = new Heapfile();
        init_heapfile(heap, page_size, column_heapfile);

        Page page;
        init_fixed_len_page(&page, page_size, record_size);

        Record *col_record = new Record();
        for (size_t j = 0; j < all_records->size(); ++j){
            col_record->push_back(all_records->at(j).at(i));

            if (col_record->size() == NUM_ATTRIBUTES) {
                // record is full, add to page
                int slot_index = add_fixed_len_page(&page, col_record);
                if (slot_index == -1) {  // page is full
                    // write page to disk
                    PageID pid = alloc_page(heap);
                    write_page(&page, heap, pid);

                    init_fixed_len_page(&page, page_size, record_size);
                    slot_index = add_fixed_len_page(&page, col_record);
                }

                write_fixed_len_page(&page, slot_index, col_record);
                col_record->clear();
            }
        }

        if (page.used_slots > 0) {
            PageID pid = alloc_page(heap);
            write_page(&page, heap, pid);
        }

        delete col_record;
        delete heap;
        fclose(column_heapfile);
    }

    ftime(&t);
    long total_run_time = ((t.time * 1000) + t.millitm) - start_time_in_ms;

    if (show_output) {
        std::cout << "TOTAL TIME: " << total_run_time << " milliseconds\n";
        std::cout << "NUMBER OF RECORDS: " << total_records << "\n";
    }

    return 0;
}
