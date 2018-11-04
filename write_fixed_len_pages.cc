#include <stdlib.h>
#include <fstream>
#include <sys/timeb.h>
#include <iostream>
#include <cstring>
#include "library.h" 
#include <sstream>

int main(int argc, const char *argv[]) {
    if (argc < 4){
        std::cout << "Error Usage. USAGE: ./write_fixed_len_pages <csv_file> <page_file> <page_size>";
        return 1;
    }
    // Initializing variables
    Page page;
    std::ofstream FilePage;
    std::string l;
    int Page_no, record_no;
    Page_no  = record_no = 0;


    std::string file_csv(argv[1]);
    std::string pageFilename(argv[1]);
    int pageSize = std::stoi(argv[3]);

    
    FilePage.open(pageFilename, std::ios::binary | std::ios::out);
    std::ifstream csv_file(file_csv);

    int new_page_required = 1;

    struct timeb t;
    ftime(&t);
    unsigned long start_ms = t.time * 1000 + t.millitm;

    while(std::getline(csv_file,l)){
        Record record;
        std::stringstream linestr(l);
        std::string cell;
        while(std::getline(linestr, cell, ',')){
            char *temp = (char *)malloc(11);
            std::strncpy(temp, cell.c_str(), 11);
            record.push_back(temp);
        }

        if (new_page_required){
            init_fixed_len_page(&page,pageSize, fixed_len_sizeof(&record));
            Page_no++;
        }
        new_page_required = add_fixed_len_page(&page, &record) == -1;
        record_no++;
        if (new_page_required){
            FilePage.write((const char *) page.data, page.page_size);
            init_fixed_len_page(&page, pageSize, fixed_len_sizeof(&record));
            add_fixed_len_page(&page,&record);
            new_page_required = 0;
            Page_no ++;
        }
    }

    if (!new_page_required){
        FilePage.write((const char *) page.data, page.page_size);
    }

    FilePage.close();

    ftime(&t);
    unsigned long stop_ms = t.time * 1000 + t.millitm;

    std::cout << "Number of record are : " << record_no << "\n";
    std::cout << " Number of page are : " << Page_no << "\n";
    std::cout << "Time: " << stop_ms - start_ms << " milliseconds\n";

    return 0;


}