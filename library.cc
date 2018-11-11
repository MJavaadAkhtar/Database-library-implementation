#include <stdlib.h>
#include "string.h"
#include <iostream>
#include <stdio.h>
#include <vector>

#include "library.h"


int fixed_len_sizeof(Record *record) {
    int size = 0;
    for (Record::iterator it = record->begin(); it != record->end(); ++it) {
        size += strlen(*it);
    }
    return size;
    // return ATTRIBUTE_SIZE * NUM_ATTRIBUTES;
}

void fixed_len_write(Record *record, char *buf) {
    for (Record::iterator it = record->begin(); it != record->end(); ++it) {
        strcat(buf, *it);
    }
}

void fixed_len_read(char *buf, int size, Record *record) {
    for (int i = 0; i < size/ATTRIBUTE_SIZE; ++i) {
        char* attribute = new char[ATTRIBUTE_SIZE + 1];
        int attribute_index = i * ATTRIBUTE_SIZE;
        strncpy(attribute, buf + attribute_index, ATTRIBUTE_SIZE);

        // not sure why garbage values are ending up in 'attribute'
        attribute[ATTRIBUTE_SIZE] = '\0';

        if (strlen(attribute) > 0) {
            record->push_back(attribute);
        }
    }
}

void init_fixed_len_page(Page *page, int page_size, int slot_size) {
    page->page_size = page_size;
    page->slot_size = slot_size;
    page->used_slots = 0;
    std::vector<Record> *data = new std::vector<Record>();

    int i;
    for (i = 0; i < fixed_len_page_capacity(page); ++i) {
        Record *r = new Record();
        data->push_back(*r);
        delete r;
    }
    page->data = data;
}

int fixed_len_page_capacity(Page *page) {
    return page->page_size / page->slot_size;
}

int fixed_len_page_freeslots(Page *page) {
    return fixed_len_page_capacity(page) - page->used_slots;
}

int add_fixed_len_page(Page *page, Record *r) {
    int slot = 0;
    std::vector<Record> records = *(page->data);
    for (std::vector<Record>::iterator it = records.begin(); it != records.end(); ++it) {
        if (it->empty()) {
            return slot;
        }
        slot ++;
    }
    return -1;
}

void write_fixed_len_page(Page *page, int slot, Record *r) {
    // only increase count if prev is empty and new is not empty
    if (page->data->at(slot).empty() && !r->empty()) {
        page->used_slots++;
    }
    page->data->at(slot) = *r;
}

void read_fixed_len_page(Page *page, int slot, Record *r) {
    *r = page->data->at(slot);
}

int number_of_pages_per_directory_page(int page_size) {
    return (page_size - sizeof(int)) / sizeof(DirectoryEntry);
}

int get_directory_number(PageID pid, int page_size) {
    return pid / (number_of_pages_per_directory_page(page_size) + 1);
}

/*
 * Moves *file to the beginning of the nth directory, where n = directory_number
 * Assumes *file has at least n directory pages
 */
void go_to_directory_by_directory_number(int directory_number, FILE *file) {
    fseek(file, 0, SEEK_SET);
    int offset_to_next_directory = 0;
    for (int i = 0; i < directory_number; ++i) {
        fread(&offset_to_next_directory, sizeof(int), 1, file);
        fseek(file, offset_to_next_directory, SEEK_CUR);
    }
}

/*
 * Scans the directory for pid
 * Assumes *file is at the top of a directory
 * Moves the file pointer to the location of pid in the directory
 * Returns 1 if successful, -1 if not
 */
int search_directory(Heapfile *heapfile, PageID pid) {
    fseek(heapfile->file_ptr, sizeof(int), SEEK_CUR);  // skip the next directory offset
    int p;
    for (int i = 0; i < number_of_pages_per_directory_page(heapfile->page_size); ++i) {
        fread(&p, sizeof(int), 1, heapfile->file_ptr);
        if (p == pid) {
            // file pointer is now pointing to freespace of matching pid
            return 1;
        }
        fseek(heapfile->file_ptr, sizeof(int), SEEK_CUR);
    }
    return -1;
}

void init_heapfile(Heapfile *heapfile, int page_size, FILE *file) {
    heapfile->file_ptr = file;
    heapfile->page_size = page_size;

    // first 8 bytes is the offset to next page
    int offset_to_next_directory_page = 0;
    fwrite(&offset_to_next_directory_page, sizeof(int), 1, file);

    // fill in the rest of the directory with empty entries
    for (int i = 1; i <= number_of_pages_per_directory_page(page_size); ++i) {
        // page_offset, starts at 1 because offset 0 is the directory page
        fwrite(&i, sizeof(int), 1, file);

        // freespace (initially set to page_size)
        fwrite(&page_size, sizeof(int), 1, file);
    }
    fflush(file);
}

int get_offset_to_last_directory_page(FILE *file) {
    fseek(file, 0, SEEK_SET);
    int offset_to_next_directory_page = 0;
    int total_offset_to_last_directory_page = 0;

    while (offset_to_next_directory_page != 0) {
        // read offset, will be 0 if current direcory is the last in the file
        fread(&offset_to_next_directory_page, sizeof(int), 1, file);

        total_offset_to_last_directory_page += offset_to_next_directory_page;

        // jump to next directory
        fseek(file, total_offset_to_last_directory_page, SEEK_SET);
    }

    return total_offset_to_last_directory_page;
}

PageID alloc_page(Heapfile *heapfile) {
    int offset_to_last_directory = get_offset_to_last_directory_page(heapfile->file_ptr) * heapfile->page_size;
    fseek(heapfile->file_ptr, offset_to_last_directory + sizeof(int), SEEK_SET);

    int page_offset = 0;
    int freespace = 0;
    for (int i = 0; i < number_of_pages_per_directory_page(heapfile->page_size); ++i) {
        fread(&page_offset, sizeof(int), 1, heapfile->file_ptr);
        fread(&freespace, sizeof(int), 1, heapfile->file_ptr);

        if (freespace == heapfile->page_size) {
            return page_offset;
        }
    }

    // if you got this far, directory is full and you need a new one
    fseek(heapfile->file_ptr, 0, SEEK_END);

    // write offset of new directory into the previous
    int current_position = ftell(heapfile->file_ptr);
    int offset_to_new_directory = current_position - offset_to_last_directory;
    fseek(heapfile->file_ptr, offset_to_last_directory, SEEK_SET);
    fwrite(&offset_to_new_directory, sizeof(int), 1, heapfile->file_ptr);

    fseek(heapfile->file_ptr, 0, SEEK_END);

    // Write an empty directory
    int next_directory_offset = 0;
    fwrite(&next_directory_offset, sizeof(int), 1, heapfile->file_ptr);
    page_offset = 0;
    for (int i = 1; i <= number_of_pages_per_directory_page(heapfile->page_size); ++i) {
        // offset is # pages from top of file
        page_offset = (current_position / heapfile->page_size) + i;
        fwrite(&page_offset, sizeof(int), 1, heapfile->file_ptr);

        // freespace = page_size
        fwrite(&heapfile->page_size, sizeof(int), 1, heapfile->file_ptr);
    }

    // Append empty page to file
    fseek(heapfile->file_ptr, 0, SEEK_END);
    PageID new_page_offset = ftell(heapfile->file_ptr);
    char *new_page = new char[heapfile->page_size];
    memset(new_page, 0, heapfile->page_size*sizeof(char));
    fwrite(new_page, heapfile->page_size, 1, heapfile->file_ptr);
    fflush(heapfile->file_ptr);

    delete[] new_page;

    return new_page_offset / heapfile->page_size;
}

void write_page(Page *page, Heapfile *heapfile, PageID pid) {
    // write page to disk
    fseek(heapfile->file_ptr, pid * heapfile->page_size, SEEK_SET);
    char *buf = new char[heapfile->page_size];
    buf[0] = '\0'; // cut off garbage values
    for (int i = 0; i < fixed_len_page_capacity(page); ++i) {
        fixed_len_write(&(page->data)->at(i), buf);
    }
    fwrite(buf, heapfile->page_size, 1, heapfile->file_ptr);

    delete buf;

    // find directory containing this page's entry, and update the freespace
    int directory_number = get_directory_number(pid, heapfile->page_size);
    go_to_directory_by_directory_number(directory_number, heapfile->file_ptr);
    int found = search_directory(heapfile, pid);
    if (!found) {
        throw;  // RIP
    }
    int freespace = heapfile->page_size - (page->used_slots * NUM_ATTRIBUTES * ATTRIBUTE_SIZE);
    fwrite(&freespace, sizeof(int), 1, heapfile->file_ptr);
    fflush(heapfile->file_ptr);
}

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page) {
    init_fixed_len_page(page, heapfile->page_size, NUM_ATTRIBUTES * ATTRIBUTE_SIZE);
    fseek(heapfile->file_ptr, pid*heapfile->page_size, SEEK_SET);

    // Create a buffer and fill it with the content of the page
    char *buf = new char[heapfile->page_size];
    fread(buf, heapfile->page_size, 1, heapfile->file_ptr);

    // Iterate over the records in the page and append to page in memory
    for (int i = 0; i < fixed_len_page_capacity(page); ++i) {
        Record *r = new Record();
        fixed_len_read(&(*(buf + i * NUM_ATTRIBUTES * ATTRIBUTE_SIZE)), NUM_ATTRIBUTES * ATTRIBUTE_SIZE, r);
        write_fixed_len_page(page, i, r);
        delete r;
    }

    delete buf;
}
