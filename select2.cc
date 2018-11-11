#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include "library.h"

int main(int argc, char** argv) {
    if (argc < 6) {
        std::cout << "Error, usage must be:\n";
        std::cout << "./select2 <colstore_name> <attribute_id> <start> <end> <page_size>\n";
        return 1;
    }

    bool show_output = true;
    if (argc == 7 && strcmp(argv[6], "--benchmark-mode") == 0) {
        show_output = false;
    }

    std::ofstream null_out("/dev/null");

    char *colstore_directory = argv[1];
    int attribute_id = atoi(argv[2]);
    if (attribute_id < 0 || attribute_id >= NUM_ATTRIBUTES) {
        std::cout << "Error, attribute_id is " << attribute_id << ", but must be between 0 and 99\n";
        return 1;
    }

    std::ostringstream path_stream;
    path_stream << colstore_directory << "/" << attribute_id;
    std::string tmp = path_stream.str();
    const char* path = tmp.c_str();

    FILE *column_heapfile = fopen(path, "r+");
    if (!column_heapfile) {
        std::cout << "Error, could not open file " << path << "\n";
        return 1;
    }

    char *start = argv[3];
    char *end = argv[4];

    int page_size = atoi(argv[5]);

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = column_heapfile;

    int num_pages_in_directory = number_of_pages_per_directory_page(page_size);

    int next_directory_offset = 0;
    fread(&next_directory_offset, sizeof(int), 1, column_heapfile);

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
            fread(&page_offset, sizeof(int), 1, column_heapfile);
            fread(&freespace, sizeof(int), 1, column_heapfile);
            if (freespace != page_size) {
                // jump to page
                current_directory_position = ftell(column_heapfile);

                read_page(heap, page_offset, &page);

                for (int j = 0; j < fixed_len_page_capacity(&page); ++j) {
                    Record r = page.data->at(j);

                    if (r.empty()) {
                        continue;
                    }

                    for (Record::iterator it = r.begin(); it != r.end(); ++it) {
                        if (strncmp(*it, start, 5) >= 0 && strncmp(*it, end, 5) <= 0) {
                            num_matching_records++;
                            char *output_substring = new char[6];
                            strncpy(output_substring, *it, 5);
                            output_substring[5] = '\0';

                            if (show_output) {
                                std::cout << output_substring << "\n";
                            } else {
                                null_out << output_substring << "\n";
                            }
                            delete output_substring;
                        }
                    }
                }

                // return to directory location
                fseek(column_heapfile, current_directory_position, SEEK_SET);
            }
        }
    }

    // one more time for last directory
    current_directory_position = ftell(column_heapfile);
    fseek(column_heapfile, 0, SEEK_END);
    if (ftell(column_heapfile) - (current_directory_position + page_size) >= 0) {
        fseek(column_heapfile, current_directory_position, SEEK_SET);
        for (int i = 0; i < num_pages_in_directory; ++i) {
            fread(&page_offset, sizeof(int), 1, column_heapfile);
            fread(&freespace, sizeof(int), 1, column_heapfile);
            if (freespace != page_size) {
                // jump to page
                current_directory_position = ftell(column_heapfile);

                read_page(heap, page_offset, &page);

                for (int j = 0; j < fixed_len_page_capacity(&page); ++j) {
                    Record r = page.data->at(j);

                    if (r.empty()) {
                        continue;
                    }

                    for (Record::iterator it = r.begin(); it != r.end(); ++it) {
                        if (strncmp(*it, start, 5) >= 0 && strncmp(*it, end, 5) <= 0) {
                            num_matching_records++;
                            char *output_substring = new char[6];
                            strncpy(output_substring, *it, 5);
                            output_substring[5] = '\0';

                            if (show_output) {
                                std::cout << output_substring << "\n";
                            } else {
                                null_out << output_substring << "\n";
                            }
                            delete output_substring;
                        }
                    }
                }

                // return to directory location
                fseek(column_heapfile, current_directory_position, SEEK_SET);
            }
        }
    }

    ftime(&t);
    long total_run_time = ((t.time * 1000) + t.millitm) - start_time_in_ms;

    fclose(column_heapfile);

    std::cout << "TOTAL TIME: " << total_run_time << " milliseconds\n";
    std::cout << "NUMBER OF MATCHING RECORDS: " << num_matching_records << "\n";

    delete heap;
    return 0;
}
