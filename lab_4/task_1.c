#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>



/**
 * Print content.
 * 
 * A function which searches in a directory and
 * prints out the file name and inode number for each
 * file in the directory.
 *
 * Example folder calls:
 * "."
 * "./test_folder"
 * "./test_folder/nested_folder_1"
 * "./test_folder/nested_folder_2"
 * "./test_folder/nested_folder_2/nested_folder_3"
 */
int print_content(char path[]){

    DIR *dir;
    struct dirent *entry;   // Directory entry buffer.
    struct stat file_stat;  // File stat buffer.
    char full_path[1024];   // Full path buffer.

    // Open directory
    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    // Read and print the entries
    // Continue while theres more entries...
    while ((entry = readdir(dir)) != NULL) {

        // Create full path
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Get file statistics
        if (stat(full_path, &file_stat) == -1) { // Get file attributes and store in buffer
            perror("stat");
            continue;
        }

        // Check if the entry is a directory base on file attributes
        if (S_ISDIR(file_stat.st_mode)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { // Skip "." and ".."
                continue;
            }
            // Make recursive call
            print_content(full_path);
        } else {
            printf("File: %s, Inode: %ld\n", entry->d_name, file_stat.st_ino);
        }
    }

    // Close the directory
    closedir(dir);
}



int main(){
    print_content(".");
    return 0;
}

