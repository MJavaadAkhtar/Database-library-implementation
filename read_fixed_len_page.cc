#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <sys/timeb.h>
#include "library.h"
#include <string>

int main(int argc, const char * argv[]){
    if (argc<3){
        std::cout << "USAGE: read_fixed_len_page <page_file> <page_size>";
        return 1;
    }
    int record_no, size_page, page_no;

    std::string filename(argv[1]);
    size_page = std::stoi(argv[2]);

    // start timer
    struct timeb t;
    ftime(&t);
    unsigned long start_ms = t.time * 1000 + t.millitm;

    std::ifstream filePage;
    filePage.open(filename, std::ios::in|std::ios::binary);

    FILE * file = fopen("tuples2.csv", "w");

    while(!filePage.eof()){
        Page page;

        init_fixed_len_page(&page, size_page, 1000);

        filePage.read((char *) page.data, size_page);

        int i=0;
        while(i!=fixed_len_page_capacity(&page)){
            Record record;
            read_fixed_len_page(&page, i, &record);
            unsigned int j=0;
            while(j != record.size()){
                std::cout << record.at(j);
                if (j!=record.size() - 1){
                    std::cout << ",";
                }
                j++;
            }
            fputs("\n", file);
            record_no++;
        }
        page_no++;
    }
    fclose(file);
    filePage.close();
    ftime(&t);
    unsigned long stop_ms = t.time * 1000 + t.millitm;

    std::cout << "Number of Records: " << record_no << "\n";
    std::cout << "Number of Pages: " << page_no << "\n";
    std::cout << "Time: " << stop_ms - start_ms << " milliseconds\n";

    return 0;
}