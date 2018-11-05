#include <sys/stat.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sys/timeb.h>

int main(int argc, char *argv[]){
    if (argc<4){
        std::cout << "USAGE: ./csv2colstore <csv_file> <colstore_name> <page_size>";
        return 1;
    }

    char *dir_colstore;
    int size_page, size_record, total_record;

    std::ifstream csv_file;
    csv_file.open(argv[1]);
    if (!csv_file){
        std::cout << "Not able to open file " << argv[1] << " line 20 \n";
        return 1;
    }

   *dir_colstore = argv[2];
   if (mkdir(dir_colstore, 0700) !== 0){
       std :: cout << "Not able to open file " << dir_colstore << "\n";
       return 1;
   } 
    size_record = NUM_ATTRIBUTES * ATTRIBUTE_SIZE;
    size_page = atoi(argv[3]);

    std::vector<Record> *r = new std::vector<Reocrd>();
    
    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;
    
}
