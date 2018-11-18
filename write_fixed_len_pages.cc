#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/timeb.h>
#include <cstring>
#include "library.h"

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        std::cout << "Usage: write_fixed_len_pages <csv_file> <page_file> <page_size>";
        return 1;
    }
    std::string csv_filename(argv[1]);
    std::string page_filename(argv[2]);
    int page_size = std::stoi(argv[3]); // 4096 for example

    // Open the page file for writing
    std::ofstream page_file;
    page_file.open(page_filename, std::ios::out | std::ios::binary);

    // Read the CSV file line-by-line:
    std::ifstream csv_file(csv_filename);
    std::string line;
    Page page;
    int should_create_new_page = 1;

    // start timer
    struct timeb t;
    ftime(&t);
    unsigned long start_ms = t.time * 1000 + t.millitm;

    // for output
    int number_of_records = 0;
    int number_of_pages = 0;

    while (std::getline(csv_file, line)) {
        std::stringstream linestr(line);
        std::string cell;

        // Read cells into a Record
        Record record;
        while (std::getline(linestr, cell, ',')) {
            char *temp = (char *) malloc(11);
            std::strncpy(temp, cell.c_str(), 11);
            record.push_back(temp);
        }

        // First run, the page will not be initialized
        if (should_create_new_page) {
            init_fixed_len_page(&page, page_size, fixed_len_sizeof(&record));
            number_of_pages += 1;
        }
        should_create_new_page = add_fixed_len_page(&page, &record) == -1;
        number_of_records += 1;

        // if -1, init a new page and add this record to it
        if (should_create_new_page) {
            // Write page.data to page_file
            page_file.write((const char *) page.data, page.page_size);

            init_fixed_len_page(&page, page_size, fixed_len_sizeof(&record));
            add_fixed_len_page(&page, &record);
            should_create_new_page = 0;
            number_of_pages += 1;
        }
    }

    if (!should_create_new_page) {
        // Write page.data to page_file
        page_file.write((const char *) page.data, page.page_size);
    }

    page_file.close();

    // stop timer
    ftime(&t);
    unsigned long stop_ms = t.time * 1000 + t.millitm;

    std::cout << "NUMBER OF RECORDS: " << number_of_records << "\n";
    std::cout << "NUMBER OF PAGES: " << number_of_pages << "\n";
    std::cout << "TIME: " << stop_ms - start_ms << " milliseconds\n";

    return 0;
}
