#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "library.h"

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Error, usage must be:\n";
        std::cout << "./insert <heapfile> <csv_file> <page_size>\n";
        return 1;
    }

    std::ifstream csv_file;
    csv_file.open(argv[2]);
    if (!csv_file) {
        std::cout << "Error, could not find file " << argv[2] << "\n";
        return 1;
    }

    FILE *heapfile = fopen(argv[1], "r+");
    if (!heapfile)
    {
        std::cout << "Error, could not find file " << argv[1] << "\n";
        return 1;
    }
    int page_size = atoi(argv[3]);
    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = heapfile;

    int record_size = 100 * 10;

    char *buffer;
    Page page;
    init_fixed_len_page(&page, page_size, record_size);

    while (csv_file) {
        std::string line;
        csv_file >> line;

        if (line.size() == 0) {
            continue;
        }
        line.erase(std::remove(line.begin(), line.end(), ','), line.end());

        Record *r = new Record;
        fixed_len_read((char*)line.c_str(), record_size, r);

        int slot_index = add_fixed_len_page(&page, r);

        if (slot_index == -1) {  // page is full
            int buf_size = page.page_size * record_size;
            buffer = new char[buf_size];

            std::vector<Record> *page_data = page.data;
            for (int i = 0; i < fixed_len_page_capacity(&page); ++i) {
                fixed_len_write(&page_data->at(i), buffer);
            }

            PageID new_pid = alloc_page(heap);
            write_page(&page, heap, new_pid);

            init_fixed_len_page(&page, page_size, record_size);
            slot_index = add_fixed_len_page(&page, r);
        }

        write_fixed_len_page(&page, slot_index, r);
    }

    if (page.used_slots > 0) {
        int buf_size = page.page_size * record_size;
        buffer = new char[buf_size];

        std::vector<Record> *page_data = page.data;
        for (int i = 0; i < fixed_len_page_capacity(&page); ++i) {
            fixed_len_write(&page_data->at(i), buffer);
        }

        PageID new_pid = alloc_page(heap);
        write_page(&page, heap, new_pid);
    }

    csv_file.close();
    fclose(heapfile);
    return 0;
}
