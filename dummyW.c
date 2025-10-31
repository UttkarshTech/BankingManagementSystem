#include "genutilsX.h"


int main() {
    // 1. Initialize an array of structs with your data
    struct User records_to_write[] = {
        {"abhi", "jeet", "Abhijeet", "Rai", "jua001", 1, 0, RC, 100000},
        {"abhii", "guptil", "Abhijeet", "Gupta", "gup001", 0, 0, RC, 140000},
        {"uttu", "9159", "Uttkarsh", "Ranjan", "ut001", 1, 0, RA, 100000},
        {"nik", "garg", "Nikhil", "Garg", "garg001", 1, 0, RM, 100000},
        {"abha", "tiw", "Abhash", "Tiwari", "tiwa001", 1, 0, RE, 100000}
    };
    
    // Calculate how many records are in the array
    int num_records = sizeof(records_to_write) / sizeof(struct User);

    // 2. Open the file
    FILE *file_ptr;
    // "wb" means "write binary"
    file_ptr = fopen(USERDETAILSFILE, "wb"); 

    if (file_ptr == NULL) {
        perror("Error opening file");
        return 1; // Exit with an error
    }

    // 3. Write the data
    // We write 'num_records' items, each with the size of 'struct Record',
    // from the 'records_to_write' array into the 'file_ptr'.
    size_t items_written = fwrite(records_to_write, sizeof(struct User), num_records, file_ptr);

    if (items_written != num_records) {
        printf("Error: Only wrote %zu out of %d records.\n", items_written, num_records);
        fclose(file_ptr);
        return 1;
    }

    // 4. Close the file
    fclose(file_ptr);

    printf("Successfully wrote %d records to %s.\n", num_records, USERDETAILSFILE);
    return 0; // Success
}