#include <algorithm>
#include <stdint.h>

#include "library.h"
#include <assert.h>
#include <stdlib.h>

#include <iostream>

#include <vector>
#include "string.h"

/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record){
    int length = 0;
    for (Record::iterator t = record->begin(); t!=record->end(); ++t){
        length += strlen(*t);
    }
    return length;
}
/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf){
    for (int i=0;i<record->size(); i++){
        int len_str = std::strlen(record->at(i));
        int pos = (len_str * i);
        std::memcpy((char *) buf + pos, record->at(i), len_str);
    }
}

/**
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size){
    int slot = page_size/slot_size;

    page->slot_size = slot_size;
    page->page_size = page_size;
    page->slot_used = 0;
    std::vector<Record> *data = new std::vector<Record>();

    for (int i=0; i<slot; ++i){
        Record *new_rec = new Record();
        data->push_back(*new_rec);
        delete new_rec;
    }
    page->data = data;
}

/**
 * Add a record to the page
 * Returns:
 *   record slot offset if successful,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r){
    int slot = 0;
    std::vector<Record> records = *(page->data);
    for (std::vector<Record>::iterator it = records.begin(); it != records.end(); ++it)
    {
        if (it->empty())
        {
            return slot;
        }
        slot++;
    }
    return -1;
}

/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r){
    *r = page->data->at(slot);
}

/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page){
    return page->page_size/page->slot_size;
}