#include <stdlib.h>
#include <fstream>
#include <sys/timeb.h>
#include <iostream>
#include "library.h" 

int main(int argc, const char *argv[]) {
    if (argc < 4){
        std::cout << "Error Usage. USAGE: ./write_fixed_len_pages <csv_file> <page_file> <page_size>"
        return 1;
    }
    std::string file_csv(argv[1]);
    std::string pageFilename(argv[1]);
    int pageSize = std::stoi(argv[3]);

    std::ifstream 

}