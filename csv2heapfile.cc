#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/timeb.h>
#include <cstring>
#include "library.h"

int main(int argc, const char * argv[]){
    if (argc<4){
        std::cout << "csv2heapfile <csv_file> <heapfile> <page_size>";
        return -1;
    }
    int size_page,new_page, record_no, page_no;

    std::string filename(argv[1]);
    std::string heapfile(argv[2]);
    size_page = std::stoi(argv[3]);
    

}