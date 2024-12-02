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
int page_table[MAX_NUM_OF_PAGES];

// TLB
// A 2D array of size 16x2, where the first column is the page number
// and the second column is the frame number.
int tlb[TLB_SIZE][2];

// Physical memory
// A 2D array of size 256x256, where the first column is the frame number
// and the second column is the value stored in that frame.
int physical_memory[NUM_OF_FRAMES][FRAME_SIZE];

// Statistic variables
int num_addresses = 0;
int page_faults = 0;
int tlb_hits = 0;
int tlb_misses = 0;
int tlb_cur_size = 0;


/**
 * Populate page table.
 * 
 * @param val: An array of 3 integers [virtual address, physical address, value]
 * @return void
 */
void populate_page_table(const int val[3]) {
    // Implement me

    // Calculate frame number.
    int page_number = floor(val[0] / FRAME_SIZE);
    int frame_number = floor(val[1] / FRAME_SIZE);

    // Populate page table row with frame no.
    page_table[page_number] = frame_number;
}



/**
 * Populate physical memory.
 * 
 * @param val: An array of 3 integers [virtual address, physical address, value]
 * @return void
 * 
 * ### NOTES ###
 * - const int[3] => [virutal address, physical , value]
 */
void populate_physical_memory(const int val[3]) {
    // Calculate frame number.
    int frame_number = floor(val[1] / FRAME_SIZE);
    // Calculate offset (using modulo).
    int offset = val[1] % FRAME_SIZE;
    // Store value in physical memory.
    physical_memory[frame_number][offset] = val[2];
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
        populate_page_table(val);
        populate_physical_memory(val);
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
int lookup_page_table(int page_num, int page_offset){
    int frame_num = page_table[page_num];
    return frame_num;
}



/**
 * Lookup physical memory.
 * 
 * @param frame_num: The frame number.
 * @param page_offset: The page offset.
 * @return int: The value stored in the physical memory.
 */
int lookup_physical_memory(int frame_num, int page_offset){
    return physical_memory[frame_num][page_offset];
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
    //int rows_occupied = sizeof(tlb) / sizeof(tlb[0]);
    int size = TLB_SIZE;
    while(i < size){
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
    }
    

    //printf("FIFO: Added page %d -> frame %d at index %d\n", page_num, frame_num, (tlb_head - 1 + TLB_SIZE) % TLB_SIZE);
}


/**
 * Lookup function.
 * 
 * @param physical_address: The physical address to look up.
 * @return int: The value stored in the physical memory.
 */
int lookup(const int physical_address) {
    // Get page number and page offset.
    int page_num = floor(physical_address / PAGE_SIZE);
    int page_offset = physical_address % PAGE_SIZE;
    // Get frame num via TLB or page table.
    int frame_num = check_TLB(page_num);
    if (frame_num == -1) {
        //printf("lookup: Error: Entry not found in TLB\n");
        frame_num = lookup_page_table(page_num, page_offset);
        // Perform FIFO on TLB.
        fifo(page_num, frame_num);
        page_faults++;        
    }

    // Return value from physical memory.
    return lookup_physical_memory(frame_num, page_offset);
}



/**
 * Lookup file.
 * 
 * @param filename: The name of the file to read from.
 * @return void
 */
void lookup_file(const char *filename) {
    // Open file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "lookup_file: Error: Cannot open file %s\n", filename);
        exit(1);
    }
    printf("lookup_file: Opened file %s\n", filename);

    // Read file
    char line[128];             // Buffer to store each line
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        // Split line into tokens on each space
        char *token = strtok(line, " ");
        // Select the values from the token.
        while (token != NULL) {
            // Get logical address and lookup value
            int logical_address = atoi(token);
            printf("%d >> %d\n", logical_address, lookup(logical_address));
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
    //printf("lookup: main: lookup(53683): %d\n", lookup(53683));

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

    return 0;
}

