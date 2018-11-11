#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include "library.h"

int main(int argc, char** argv) {
    if (argc < 6) {
        std::cout << "Error, usage must be:\n";
        std::cout << "./select <heapfile> <attribute_id> <start> <end> <page_size>\n";
        return 1;
    }

    bool show_output = true;
    if (argc == 7 && strcmp(argv[6], "--benchmark-mode") == 0) {
        show_output = false;
    }

    std::ofstream null_out("/dev/null");

    FILE* heapfile = fopen(argv[1], "r");
    if (!heapfile) {
        std::cout << "Error, could not find file " << argv[1] << "\n";
        return 1;
    }

    int attribute_id = atoi(argv[2]);
    if (attribute_id < 0 || attribute_id >= NUM_ATTRIBUTES) {
        std::cout << "Error, attribute_id is " << attribute_id << ", but must be between 0 and 99\n";
        return 1;
    }

    char *start = argv[3];
    char *end = argv[4];

    int page_size = atoi(argv[5]);

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = heapfile;

    int num_pages_in_directory = number_of_pages_per_directory_page(page_size);

    int next_directory_offset = 0;
    fread(&next_directory_offset, sizeof(int), 1, heapfile);

    int page_offset = 0 ;
    int freespace = 0;
    int current_directory_position = 0;
    int num_matching_records = 0;

    Page page;

    struct timeb t;
    ftime(&t);
    long start_time_in_ms = (t.time * 1000) + t.millitm;

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

                    if (r.empty()) {
                        continue;
                    }

                    if (strcmp(r.at(attribute_id), start) >= 0 && strcmp(r.at(attribute_id), end) <= 0) {
                        num_matching_records++;
                        char *output_substring = new char[6];
                        strncpy(output_substring, r.at(attribute_id), 5);
                        output_substring[5] = '\0';

                        if (show_output) {
                            std::cout << output_substring << "\n";
                        } else {
                            null_out << output_substring << "\n";
                        }
                        delete output_substring;
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

                    if (r.empty()) {
                        continue;
                    }

                    if (strncmp(r.at(attribute_id), start, 5) >= 0 && strncmp(r.at(attribute_id), end, 5) <= 0) {
                        num_matching_records++;
                        char *output_substring = new char[6];
                        strncpy(output_substring, r.at(attribute_id), 5);
                        output_substring[5] = '\0';

                        if (show_output) {
                            std::cout << output_substring << "\n";
                        } else {
                            null_out << output_substring << "\n";
                        }

                        delete output_substring;
                    }
                }

                // return to directory location
                fseek(heapfile, current_directory_position, SEEK_SET);
            }
        }
    }

    ftime(&t);
    long total_run_time = ((t.time * 1000) + t.millitm) - start_time_in_ms;

    fclose(heapfile);

    std::cout << "TOTAL TIME: " << total_run_time << " milliseconds\n";
    std::cout << "NUMBER OF MATCHING RECORDS: " << num_matching_records << "\n";

    delete heap;
    return 0;
}
