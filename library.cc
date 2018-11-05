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

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file){
    heapfile -> page_size = page_size;
    headfile -> file_ptr = file;

    //We know the first 8 bytes is the offset to the next page
    int offset_dir = 0;
    fwrite(&offset_dir, sizeof(int), 1, file);

    // Filling the rest of the directory with emptines
    int temp = ((page_size - sizeof(int)) / sizeof(DirectoryEntry));
    for (int i = 1; i <= temp; ++i){
        fwrite(&i, sizeof(int), 1, file);
        fwrite(&page_size, sizeof(int),1,file);
    }
    fflush(file);
}

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record){
    int i = 0;
    while(i< size/ATTRIBUTE_SIZE){
        char *attr = new char(ATTRIBUTE_SIZE+1);
        strncpy(attr, buf + (i*ATTRIBUTE_SIZE), ATTRIBUTE_SIZE);

        attr[ATTRIBUTE_SIZE] - '\0';

        if (strlen(attr) > 0){
            record -> push_back(attr);
        }
        ++i;
    }
}

/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r){
    int temp = !r->empty() && page->data->at(slot).empty();
    if (temp){
        page->used_slots++;
    }
    page->data->at(slot) = *r;
}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid){
    fseek(heapfile->file_ptr, pid *heapfeal->page_size, SEEK_SET);
    char *temp = new char[heapfile->page_size];
    temp[0] = '\0';
    int i = 0;
    while(i<fixed_len_page_capacity(page)){
        fixed_len_write(&(page->data)->at(i), temp);
        ++i;
    }
    fwrite(temp, heapfile->page_size,1 , heapfile->file_ptr);
    delete buf;

    int dir_no = get_directory_number(pid, heapfile->page_size);
    go_to_directory_by_directory_number(dir_no, heapfile->file_ptr);
    if(search_directory(heapfile, pid)){
        throw;
    }
    
    int space = heapfile->page_size - (page->used_slots * NUM_ATTRIBUTES * ATTRIBUTE_SIZE);
    fwrite(&space, sizeof(int), 1, heapfile->file_ptr);
    fflush(heapfile->file_ptr);
}