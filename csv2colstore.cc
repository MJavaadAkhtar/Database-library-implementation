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
        std::cout << "./csv2colstore <file_csv> <colstore_name> <size_page>\n";
        return 1;
    }

    bool output_show = true;
    if (argc == 5 && strcmp(argv[4], "--no-output") == 0) {
        output_show = false;
    }

    

    char *dir_col = argv[2];
    if (mkdir(dir_col, 0700) != 0) {
        std::cout << "Error, could not create directory " << dir_col << "\n";
        return 1;
    }
    std::ifstream file_csv;
    file_csv.open(argv[1]);
    if (!file_csv)
    {
        std::cout << "Error, could not find file " << argv[1] << "\n";
        return 1;
    }

    int size_page = atoi(argv[3]);

    int record_size = 100 * 10;

    std::vector<Record> *all_records = new std::vector<Record>();

    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;

    int total_records = 0;
    // read csv into list of records
    while (file_csv) {
        std::string line;
        file_csv >> line;

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

    file_csv.close();

    // create column file for each attribute
    for (int i = 0; i < 100; ++i) {
        // column file should be /{directory_path}/{column_index}
        std::ostringstream path_stream;
        path_stream << dir_col << "/" << i;
        std::string tmp = path_stream.str();
        const char* path = tmp.c_str();

        FILE *heapFile_col = fopen(path, "w+");
        if (!heapFile_col) {
            std::cout << "Error, could not open file " << path << " for column " << i << "\n";
            return 1;
        }

        Heapfile *heap = new Heapfile();
        init_heapfile(heap, size_page, heapFile_col);

        Page page;
        init_fixed_len_page(&page, size_page, record_size);

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

                    init_fixed_len_page(&page, size_page, record_size);
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
        fclose(heapFile_col);
    }

    ftime(&t);
    long run_time = ((t.time * 1000) + t.millitm) - start_time_in_ms;

    if (output_show) {
        std::cout << "TOTAL TIME: " << run_time << " milliseconds\n";
        std::cout << "NUMBER OF RECORDS: " << total_records << "\n";
    }

    return 0;
}
