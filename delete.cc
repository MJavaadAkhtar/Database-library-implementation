#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "library.h"

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Error, usage must be:\n";
        std::cout << "./delete <heapfile> <record_id> <page_size>\n";
        return 1;
    }

    FILE *heapfile = fopen(argv[1], "r+");
    if (!heapfile) {
        std::cout << "Error, could not find file " << argv[1] << "\n";
        return 1;
    }

    std::string serialized_record_id = argv[2];
    size_t delimiter_position = serialized_record_id.find(RID_DELIMITER);
    if (delimiter_position == 0 || delimiter_position == serialized_record_id.length() - 1) {
        std::cout << "Error, record_id must be:\n";
        std::cout << "<page_id>%<slot_number>\n";
        return 1;
    }

    RecordID *rid = new RecordID();
    rid->page_id = atoi(serialized_record_id.substr(0, delimiter_position).c_str());
    rid->slot = atoi(serialized_record_id.substr(delimiter_position + 1).c_str());

    int page_size = atoi(argv[3]);

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = heapfile;

    Page page;
    // write an empty record into the slot to delete
    Record *empty_record = new Record(NUM_ATTRIBUTES, "0000000000");
    read_page(heap, rid->page_id, &page);
    if (!page.data->at(rid->slot).empty()) {
        page.used_slots --;
    }
    write_fixed_len_page(&page, rid->slot, empty_record);
    write_page(&page, heap, rid->page_id);

    fclose(heapfile);
    delete rid;
    delete empty_record;
    delete heap;
    return 0;
}
