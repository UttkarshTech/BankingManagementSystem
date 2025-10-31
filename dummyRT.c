#include "genutilsX.h"

int main() {
    FILE *file_ptr;
    struct Txn record_buffer; // A buffer to hold one record at a time

    // 1. Open the file in binary read mode ("rb")
    file_ptr = fopen(TXNSFILE, "rb");
    if (file_ptr == NULL) {
        perror("Error opening for reading");
        return 1;
    }

    printf("--- Reading records from txns.dat ---\n");

    // 2. Read one struct at a time until the end of the file
    // fread returns 1 as long as it successfully reads one full record
    while (fread(&record_buffer, sizeof(record_buffer), 1, file_ptr) == 1) {
        
        // 3. Print the record we just read
        printf("Sender : %s\nReceiver : %s\nAmount : %0.2f\nSenderBal : %0.2f\nReceiverBal : %0.2f\n", 
               record_buffer.sender, 
               record_buffer.receiver, record_buffer.amt, record_buffer.senderBalAfterTxn,
                record_buffer.receiverBalAfterTxn
            );
        printf("----------\n");
    }
    
    printf("--- End of records ---\n");

    // 4. Close the file
    fclose(file_ptr);

    return 0;
}