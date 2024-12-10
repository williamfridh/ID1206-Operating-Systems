#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/**
 * Delete old test file.
 * 
 * Used for cleaning up the directory before creating a new test file.
 */
int delete_old_test_file(const char path[]) {
    if (remove(path) == 0) {
        //printf("Deleted old test file.\n");
    } else {
        //printf("No old test file to delete.\n");
    }
    return 0;
}



/**
 * Create and write to a test file.
 * 
 * Used for writing data to a testfile with differnet sizes of application-level buffers.
 * 
 * @param path The path to the file.
 * @param i The chunk size = 2^i.
 * @param j The buffer size = 2^j.
 */
int create_test_file(const char path[], int i, int j) {
    FILE *file = fopen(path, "w");
    if (!file) {
        perror("Failed to create file");
        return -1;
    }

    // Set application-level buffer
    static char app_buffer[1 << 20]; // 1 MiB static buffer
    size_t nbuf = 1 << j;            // 2^j
    if (nbuf > sizeof(app_buffer)) {
        fprintf(stderr, "Requested buffer size too large.\n");
        fclose(file);
        return -1;
    }
    setvbuf(file, app_buffer, _IOFBF, nbuf);

    // Write data in chunks of 2^i bytes
    size_t nio = 1 << i;        // 2^i
    char chunk[nio];            // Static chunk buffer
    memset(chunk, 'A', nio);    // Fill buffer with 'A'
    chunk[nio] = '\0';          // Null-terminate the string

    // Writing to file 
    size_t total_size = 1 << 30; // 1 GiB = 2^30 bytes
    for (size_t written = 0; written < total_size; written += nio) {
        //if (fwrite(chunk, 1, nio, file) != nio) {
        if (fputs(chunk, file) == EOF) {
            perror("Failed to write to file");
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}



/**
 * Main.
 */
int main() {
    // Test file path
    const char path[] = "./test_file.txt";

    for (int j = 20; j < 21; j = j + 2) {
        //prinf("Running test with buffer size = %d\n", j);
        for (int i = 6; i <= j; i = i + 2) {
            //printf("Running test with chunk size = %d and buffer size = %d\n", i, j);

            // Start timer, call create_test_file, stop timer
            struct timeval start, end;
            gettimeofday(&start, NULL);
            for (int ii = 0; ii < 3; ii++) {
                delete_old_test_file(path);
                create_test_file(path, i, j);
            }
            gettimeofday(&end, NULL);

            // Calculate time
            long elapsed = ((end.tv_sec - start.tv_sec) * 1000000L + end.tv_usec) - start.tv_usec;

            // Print time
            //printf("Time: %ld\n\n", elapsed / 3);
            printf("(%d, %d, %ld)\n", i, j, elapsed / 3);
        }
    }
}

