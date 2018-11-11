#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "library.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Error, usage must be:\n";
        std::cout << "./scan <heapfile> <page_size>\n";
        return 1;
    }

    FILE* heapfile = fopen(argv[1], "rb");
    if (!heapfile) {
        std::cout << "Error, could not find file " << argv[1] << "\n";
        return 1;
    }

    int page_size = atoi(argv[2]);

    // std::ofstream null_out("/dev/null");

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = heapfile;

    int num_pages_in_directory = number_of_pages_per_directory_page(page_size);

    int next_directory_offset = 0;
    fread(&next_directory_offset, sizeof(int), 1, heapfile);

    int page_offset = 0;
    int freespace = 0;
    int current_directory_position = 0;

    Page page;

    while (next_directory_offset != 0) {
        for (int i = 0; i < num_pages_in_directory; ++i) {
            fread(&page_offset, sizeof(int), 1, heapfile);
            fread(&freespace, sizeof(int), 1, heapfile);
            if (freespace != page_size) {
                // jump to page
                current_directory_position = ftell(heapfile);

                read_page(heap, page_offset, &page);

                for (int j = 0; j < fixed_len_page_capacity(&page); ++j) {
                    Record r = page.data->at(j);
                    bool empty_record = true;
                    for (Record::iterator it = r.begin(); it != r.end(); ++it) {
                        if (strlen(*it) > 0) {
                            empty_record = false;
                            std::cout << *it;
                            if (it + 1 != r.end()) {
                                std::cout << ",";
                            }
                        }
                    }
                    if (!empty_record) {
                        std::cout << "\n";
                    }
                }

                // return to directory location
                fseek(heapfile, current_directory_position, SEEK_SET);
            }
        }
    }

    // one more time for last directory
    current_directory_position = ftell(heapfile);
    fseek(heapfile, 0, SEEK_END);
    if (ftell(heapfile) - (current_directory_position + page_size) >= 0) {
        fseek(heapfile, current_directory_position, SEEK_SET);
        for (int i = 0; i < num_pages_in_directory; ++i) {
            fread(&page_offset, sizeof(int), 1, heapfile);
            fread(&freespace, sizeof(int), 1, heapfile);
            if (freespace != page_size) {
                // jump to page
                current_directory_position = ftell(heapfile);

                read_page(heap, page_offset, &page);

                for (int j = 0; j < fixed_len_page_capacity(&page); ++j) {
                    Record r = page.data->at(j);
                    bool empty_record = true;
                    for (Record::iterator it = r.begin(); it != r.end(); ++it) {
                        if (strlen(*it) > 0) {
                            empty_record = false;
                            std::cout << *it;
                            if (it + 1 != r.end()) {
                                std::cout << ",";
                            }
                        }
                    }
                    if (!empty_record) {
                        std::cout << "\n";
                    }
                }

                // return to directory location
                fseek(heapfile, current_directory_position, SEEK_SET);
            }
        }
    }

    return 0;
}
