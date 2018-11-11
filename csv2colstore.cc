#include <stdlib.h>
#include <algorithm>
#include <sys/stat.h>
#include "library.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <sys/timeb.h>
#include <sstream>


int main(int argc, char** argv) {

    if (argc < 4) {
        std::cout << "USAGE: ./csv2colstore <csv_file> <colstore_name> <page_size>\n";
        return 1;
    }

    std::ifstream csv_file;
    char *colstore_directory = argv[2];
    int page_size, record_size, total_records;
    record_size = 1000;
    page_size = atoi(argv[3]);

    bool show_output = true;
    if (argc == 5 && strcmp(argv[4], "--no-output") == 0)
    {
        show_output = false;
    }

    csv_file.open(argv[1]);
    if (!csv_file) {
        std::cout << "Error, could not find file " << argv[1] << "\n";
        return 1;
    }

    if (mkdir(colstore_directory, 0700) != 0) {
        std::cout << "Error, could not create directory " << colstore_directory << "\n";
        return 1;
    }

    std::vector<Record> *all_records = new std::vector<Record>();

    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;

    total_records = 0;

    // read csv into list of records
    while (csv_file) {
        std::string line;
        csv_file >> line;
        if (line.size() == 0) {
            continue;
        }
        Record *r = new Record;
        // remove all commas from the line
        line.erase(std::remove(line.begin(), line.end(), ','), line.end());
        // turn 'line' from string to char*, and read the values into r
        fixed_len_read((char*)line.c_str(), record_size, r);
        all_records->push_back(*r);
        delete r;
        total_records++;
    }

    csv_file.close();

    // create column file for each attribute
    int i = 0;
    while(i<100){
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
        Page page;
        Record *col_record = new Record();

        init_heapfile(heap, page_size, column_heapfile);
        init_fixed_len_page(&page, page_size, record_size);

        int j = 0;
        while(j<all_records->size()){
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
            ++j;
        }

        if (page.used_slots > 0) {
            PageID pid = alloc_page(heap);
            write_page(&page, heap, pid);
        }

        delete col_record;
        delete heap;
        fclose(column_heapfile);
        i++;
    }

    ftime(&t);
    long total_run_time = ((t.time * 1000) + t.millitm) - start_time_in_ms;

    if (show_output) {
        std::cout << "TOTAL TIME: " << total_run_time << " milliseconds\n";
        std::cout << "NUMBER OF RECORDS: " << total_records << "\n";
    }

    return 0;
}
