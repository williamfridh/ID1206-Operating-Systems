/**
 * This program simulates a virtual memory system with a page table and physical memory
 * using a TLB (Translation Lookaside Buffer) to speed up the process.
 * 
 * ### NOTES ###
 * - Command to run: "gcc simulator.c -o simulator; ./simulator;"
 * - The page_table and physical_memory arrays should hold char (1 byte) values.
 * 
 * ### TODO ###
 * -
 * 
 * ### QUESTIONS ###
 * - Should the physical memory be allocated using malloc? No, it's up to us.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

int corre_counter = 0;
int correct[1000]; // tmp

// Check TLB => Not in TLB => Check Page table => Not in page table => check backing store => 


// Parameters given by the assignment
#define MAX_NUM_OF_PAGES 256
#define PAGE_SIZE 256
#define NUM_OF_FRAMES 256
#define FRAME_SIZE 256
#define TLB_SIZE 16

int tlb_head = 0;    // Points to the oldest entry (to evict next if TLB is full)
int tlb_count = 0;   // Tracks the number of entries currently in the TLB

// Page table
// A 1D array of size 256, where the index is the
// page number and the value is the frame number.
struct page_table_entry
{
    int frame_num;
    bool valid;
};
struct page_table_entry page_table[MAX_NUM_OF_PAGES];

// TLB
// A 2D array of size 16x2, where the first column is the page number
// and the second column is the frame number.
int tlb[TLB_SIZE][2];

// Physical memory & backing store
// A 2D array of size 256x256, where the first column is the frame number
// and the second column is the value stored in that frame.
int physical_memory[NUM_OF_FRAMES * FRAME_SIZE];

struct backing_store_entry
{
    int physical_address;
    int value;
};
struct backing_store_entry backing_store[100000];

// Statistic variables
int num_addresses = 0;
int page_faults = 0;
int tlb_hits = 0;
int tlb_misses = 0;
int tlb_cur_size = 0;


/**
 * Add to page table.
 * 
 * @param val: An array of 3 integers [virtual address, physical address, value]
 * @return int
 */
int add_to_page_table(const int page_num, const int frame_num) {
    // Populate page table row with frame no.
    page_table[page_num].frame_num = frame_num;
    page_table[page_num].valid = true;
    // Print success message.
    printf("add_to_page_table: Added page %d -> frame %d\n", page_num, frame_num);
    // Return frame number.
    return frame_num;
}



/**
 * Populate backing store.
 * 
 * @param val: An array of 3 integers [virtual address, physical address, value]
 * @return void
 * 
 * ### NOTES ###
 * - const int[3] => [virutal address, physical , value]
 */
void populate_backing_store(const int val[3]) {
    // Get virtual address.
    int virtual_address = val[0];
    // Get physical address.
    int physical_address = val[1];
    // Get value.
    int value = val[2];
    // Store value in physical memory.
    backing_store[virtual_address].physical_address = physical_address;
    backing_store[virtual_address].value = value;
}



/*
 * Populator.
 * Reads a file and populates the page table and physical memory.
 * 
 * @param filename: The name of the file to read from.
 * @return void
 */
void populate(const char *filename) {

    printf("populate: Reading file %s\n", filename);

    // Initialize page table valid to false.
    for (int i = 0; i < MAX_NUM_OF_PAGES; i++) {
        page_table[i].valid = false;
    }

    // Open file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "populate: Error: Cannot open file %s\n", filename);
        exit(1);
    }
    printf("populate: Opened file %s\n", filename);

    // Read file
    char line[128];             // Buffer to store each line
    while (fgets(line, sizeof(line), file)) {
        int i = 0; // Keep track of what value we are on
        int val[3]; // Array to store the values (virtual address, physical address, value)
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        // Split line into tokens on each space
        char *token = strtok(line, " ");
        // Select the values from the token.
        while (token != NULL) {
            // If token is a number, print it
            if (atoi(token) != 0 || strcmp(token, "0") == 0) {
                //printf("%d\n", atoi(token));
                // Set the value in the val array and restart the array if we have all 3 values
                val[i] = atoi(token);
                i++;
            }
            token = strtok(NULL, " "); // Get next token
        }
        // Send found values to populate_page_table and populate_physical_memory
        //populate_page_table(val); // Should be populated in the lookup function
        populate_backing_store(val);
        correct[corre_counter++] = val[2]; // tmp
        // Increment number of addresses
        num_addresses++;
    }

    // Close file
    fclose(file);
    printf("populate: Closed file %s\n", filename);
}



/**
 * Lookup page table.
 * 
 * @param log_addr: The logical address to look up.
 * @return int: The value stored in the physical memory.
 */
int lookup_page_table(int page_num, int offset){
    struct page_table_entry frame_num = page_table[page_num];
    if (frame_num.valid) {
        return frame_num.frame_num;
    } else {
        return -1;
    }
}



/**
 * Lookup physical memory.
 * 
 * @param frame_num: The frame number.
 * @param offset: The page offset.
 * @return int: The value stored in the physical memory.
 */
int lookup_physical_memory(int frame_num, int offset){
    int res = physical_memory[frame_num * FRAME_SIZE + offset];
    if (frame_num >= NUM_OF_FRAMES || offset >= FRAME_SIZE) {
        printf("lookup_physical_memory: Error: Index out of bounds\n");
        exit(1);
    }
    return res;
}



/** 
 * Checking out our TLB if it contains the, 
 * page number.
 *
 * @param page_num the page number in which we search the TLB for  
 * @return int (the physical address if found, if not -1 as it's 
 * supposed to cast an error
*/
int check_TLB(int page_num){
    int i = 0;
    int size = TLB_SIZE;
    while(i < TLB_SIZE){
        if(page_num == tlb[i][0]){
            tlb_hits++; // Increment the TLB hits
            return tlb[i][1];
        }
        i++;
    }
    tlb_misses++; // Increment the TLB misses
    return -1;
}



/**
 * FIFO TLB replacement policy.
 * 
 * Adds a new page-frame entry to the TLB. If the TLB is full,
 * evicts the oldest entry and replaces it with the new one.
 *
 * @param page_num: The page number to be added.
 * @param frame_num: The frame number to be added.
 * @return void
 */
void fifo(int page_num, int frame_num) {
    // Add the new entry to the TLB
    tlb[tlb_head][0] = page_num;  // Store the page number
    tlb[tlb_head][1] = frame_num; // Store the frame number

    // Move the tlb_head to the next position
    tlb_head = (tlb_head + 1) % TLB_SIZE;


    // Increment tlb_count if not full
    if (tlb_count < TLB_SIZE) {
        tlb_count++;
        if (tlb_count == TLB_SIZE) {
            //printf("FIFO: TLB is full\n");
        }
    }
    

    //printf("FIFO: Added page %d -> frame %d at index %d\n", page_num, frame_num, (tlb_head - 1 + TLB_SIZE) % TLB_SIZE);
}


/**
 * Adding the val from the backing store to the physical
 * memory.
 * 
 * @param frame_num: The frame number 
 * @param offset: Offset
 * @param Val: Value held in the 
 */

void add_to_phys_mem(const int frame_num, const int offset, const int val){
    physical_memory[frame_num * FRAME_SIZE + offset] = val;
}


/**
 * Lookup function.
 * 
 * @param physical_address: The physical address to look up.
 * @return int: The value stored in the physical memory.
 */
int lookup(const int virtual_address) {
    // Get page number and page offset.
    int page_num = floor(virtual_address / PAGE_SIZE);
    int offset = virtual_address % PAGE_SIZE;
    // Get frame num via TLB or page table.
    int frame_num = check_TLB(page_num);
    //printf("lookup: page_num: %d, offset: %d, frame_num: %d\n", page_num, offset, frame_num);
    if (frame_num == -1) {
        //printf("lookup: Error: Entry not found in TLB\n");
        frame_num = lookup_page_table(page_num, offset);
        // Catch page fault.
        //struct page_table_entry bool  = page_table[page_num].valid;

        if (frame_num == -1) {
            //printf("lookup: Error: Page fault\n");
            page_faults++;
            //get_frame_num();
            
            int physical_address = backing_store[virtual_address].physical_address;
            int val = backing_store[virtual_address].value;

            //printf("found physical address: %d\n", physical_address);
            //printf("found value: %d\n", val);

            frame_num = floor(physical_address / FRAME_SIZE);

            add_to_page_table(page_num, frame_num);
            add_to_phys_mem(frame_num, offset, val);

        }
        // Perform FIFO on TLB.
        fifo(page_num, frame_num);
        //return lookup(virtual_address);
    }

    printf("lookup: frame_num %d -> offset %d\n", frame_num, offset);

    // Return value from physical memory.
    return lookup_physical_memory(frame_num, offset);
}



/**
 * Lookup file.
 * 
 * @param filename: The name of the file to read from.
 * @return void
 */
void lookup_file(const char *filename) {
    corre_counter = 0;
    // Open file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "lookup_file: Error: Cannot open file %s\n", filename);
        exit(1);
    }
    printf("lookup_file: Opened file %s\n", filename);

    // Read file
    char line[256];             // Buffer to store each line
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        // Split line into tokens on each space
        char *token = strtok(line, " ");
        // Select the values from the token.
        while (token != NULL) {
            // Get logical address and lookup value
            int virtual_address = atoi(token);
            int res = lookup(virtual_address);
            printf("\n%d >> %d\n", virtual_address, res);
            if (res != correct[corre_counter++]) {
                printf("==================== should be: %d\n", correct[corre_counter++]);
            }
            token = strtok(NULL, " "); // Get next token
        }
    }
}



/**
 * Main function.
 */
int main() {
    populate("correct.txt");
    lookup_file("addresses.txt");
    //printf("lookup: main: lookup(30198): %d\n", lookup(30198));
    //printf("lookup: main: lookup(53683): %d\n", lookup(53683));
    //printf("lookup: main: lookup(12107): %d\n", lookup(12107));

    // Print out statistics
    printf("\n=========== STATISTICS ===========\n");
    printf("Number of addresses: %d\n", num_addresses);
    printf("Page faults: %d\n", page_faults);
    printf("TLB hits: %d\n", tlb_hits);
    printf("TLB misses: %d\n", tlb_misses);
    
    printf("\n=========== BY SIZE ===========\n");
    printf("size of TLB: %d\n", TLB_SIZE);
    printf("TLB hits by size: %f\n", (double)tlb_hits/TLB_SIZE);
    printf("TLB misses by size: %f\n\n", (double)tlb_misses/TLB_SIZE);

    // Print tlb
    printf("\n=========== TLB Table ===========\n");
    //for(int i = 0; i < TLB_SIZE; i++){
    //    printf("TLB: %d -> %d\n", tlb[i][0], tlb[i][1]);
    //}

    // Print backing store.
    printf("\n=========== Backing Store ===========\n");
    for (int i = 0; i < NUM_OF_FRAMES; i++) {
        for (int j = 0; j < FRAME_SIZE; j++) {
            //if (backing_store[i][j] != 0) {
                //printf("Backing Store: %d -> %d\n", i, backing_store[i][j]);
            //}
        }
    }

    // Print physical memory.
    printf("\n=========== Physical Memory ===========\n");
    for (int i = 0; i < NUM_OF_FRAMES; i++) {
        for (int j = 0; j < FRAME_SIZE; j++) {
            if (physical_memory[i * FRAME_SIZE + j] != 0) {
                //printf("Physical Memory: %d -> %d\n", i, physical_memory[i * FRAME_SIZE + j]);
            }
        }
    }

    return 0;
}

