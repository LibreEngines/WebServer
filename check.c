#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// Supported file extensions (set to NULL to check all files)
const char *valid_extensions[] = {
    ".c", ".h", ".cpp", ".hpp", ".cc", ".java", 
    ".py", ".php", ".js", ".html", ".css", ".sh", NULL
};

// Function to check if a file has a valid extension
int is_valid_file(const char *filename) {
    // If no extensions specified, check all files
    if (valid_extensions[0] == NULL) {
        return 1;
    }

    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    
    for (int i = 0; valid_extensions[i] != NULL; i++) {
        if (strcmp(dot, valid_extensions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Count lines in a single file
int count_lines_in_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    int lines = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') lines++;
    }

    fclose(file);
    return lines;
}

// Process a directory recursively
void process_directory(const char *path, int *total_files, int *total_lines) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    if ((dir = opendir(path)) == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &statbuf) == -1) {
            perror("Error getting file stats");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Recursively process subdirectory
            process_directory(full_path, total_files, total_lines);
        } else {
            // Process file
            if (is_valid_file(entry->d_name)) {
                int lines = count_lines_in_file(full_path);
                if (lines >= 0) {
                    (*total_files)++;
                    *total_lines += lines;
                    printf("%s: %d lines\n", full_path, lines);
                }
            }
        }
    }

    closedir(dir);
}

int main() {
    char current_dir[1024];
    
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("Error getting current directory");
        return EXIT_FAILURE;
    }

    int total_files = 0;
    int total_lines = 0;

    printf("Scanning current directory: %s\n", current_dir);
    process_directory(current_dir, &total_files, &total_lines);

    printf("\nSummary:\n");
    printf("Total files scanned: %d\n", total_files);
    printf("Total lines of code: %d\n", total_lines);

    return EXIT_SUCCESS;
}