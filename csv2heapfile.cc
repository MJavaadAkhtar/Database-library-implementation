#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/timeb.h>
#include <cstring>
#include "library.h"

int main(int argc, const char * argv[]){
    if (argc < 4) {
        std::cout << "USAGE: ./csv2headfile <csv_file> <heapfile> <page_size>";
        return 1;
    }

    int is_output = 1;
    int size_page, size_record, total_records;
    Page page;

    //opening a csv file using ifstream
    std::ifstream csv_file;
    csv_file.open(argv[1]);

    //if file fails to open, then return an error
    if (!csv_file){
        std::cout <<"Is not able to open the file" << argv[1] << " in line 18 \n";
        return 1;
    }

    //Initializing the heapfile and page
    Heapfile *heap = new Heapfile();
    size_page = atoi(argv[3]);
    FILE *heap_file  = fopen(argv[2], "w+b");
    if (!(heap_file)){
        std::cout << "ERROR: not able to find the file "<<argv[2]<<" in line 32\n";
        return 1;
    }
    size_record = NUM_ATTRIBUTES * ATTRIBUTE_SIZE;
    init_fixed_len_page(&page, size_page,size_record);
    init_heapfile(heap, size_page, heap_file);

    //Timing
    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;

    while(csv_file){
        Record *record;
        std::string line;
        int indexSlot;

        csv_file >> line;
        if (line.size()==0){
            continue;
        }

        //remove all commas from the line
        *record = new Record;
        line.erase(std::remove(line.begin(), line.end(), ","), line.end());

        fixed_len_read((char*)line.c_str(), size_record, r);
        indexSlot = add_fixed_len_page(&page, r);

        //IIf the page is full, write to the disk and recalculate the slot index
        if (indexSlot == -1){
            PageID id;
            id = alloc_page(&page, heap, id);
            write_page(&page, heap,id);
            init_fixed_len_page(&page, size_page, size_record);
            indexSlot = add_fixed_len_page(&page,r);
        }

        write_fixed_len_page(&page, indexSlot, r);
        total_records=+1;
    }
    
    //If there is some record, write it to the last file
    if (page.used_slot > 0){
        PageID id = alloc_page(heap);
        write_page(&page,heap, id);
    }

    //calculating the end of time
    ftime(&t);
    long total_run_time = ((t.time * 1000) + t.millitm) - start_time_in_ms;

    csv_file.close();
    std::cout << "RUN TIME Total: " << total_run_time << " milliseconds \n";
    std::cout << "TOTAL Records: " << total_records << "\n";

    return 0;

}