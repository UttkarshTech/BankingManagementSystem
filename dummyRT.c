#include "genutilsX.h"

int main() {
    FILE *file_ptr;
    struct Txn record_buffer;
    file_ptr = fopen(TXNSFILE, "rb");
    if (file_ptr == NULL) {
        perror("Error opening for reading");
        return 1;
    }

    printf("--- Reading records from txns.dat ---\n");
    while (fread(&record_buffer, sizeof(record_buffer), 1, file_ptr) == 1) {
        printf("Sender : %s\nReceiver : %s\nAmount : %0.2f\nSenderBal : %0.2f\nReceiverBal : %0.2f\n", 
               record_buffer.sender, 
               record_buffer.receiver, record_buffer.amt, record_buffer.senderBalAfterTxn,
                record_buffer.receiverBalAfterTxn
            );
        printf("----------\n");
    }
    
    printf("--- End of records ---\n");

    
    fclose(file_ptr);

    return 0;
}