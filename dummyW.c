#include "genutilsX.h"


int main() {
    
    struct User records_to_write[] = {
        {"abhi", "jeet", "Abhijeet", "Rai", "jua001", 1, 0, RC, 100000},
        {"abhii", "guptil", "Abhijeet", "Gupta", "gup001", 1, 0, RC, 100000},
        {"uttu", "9159", "Uttkarsh", "Ranjan", "ut001", 1, 0, RA, 100000},
        {"nik", "garg", "Nikhil", "Garg", "garg001", 1, 0, RM, 100000},
        {"abha", "tiw", "Abhash", "Tiwari", "tiwa001", 1, 0, RE, 100000},
        {"poo", "jan", "Poojan", "Pandya", "poo001", 1, 0, RE, 100000}
    };
    
    
    int num_records = sizeof(records_to_write) / sizeof(struct User);

    
    FILE *file_ptr;
    
    file_ptr = fopen(USERDETAILSFILE, "wb"); 

    if (file_ptr == NULL) {
        perror("Error opening file");
        return 1; 
    }
    
    size_t items_written = fwrite(records_to_write, sizeof(struct User), num_records, file_ptr);

    if (items_written != num_records) {
        printf("Error: Only wrote %zu out of %d records.\n", items_written, num_records);
        fclose(file_ptr);
        return 1;
    }

    
    fclose(file_ptr);

    printf("Successfully wrote %d records to %s.\n", num_records, USERDETAILSFILE);
    return 0; 
}