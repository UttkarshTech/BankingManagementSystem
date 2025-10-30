#include "genutilsX.h"

void safe_write(int fd, const char *msg) {
    write(fd, msg, strlen(msg));
}

int safe_read_line(int fd, char *buffer, int size) {
    ssize_t bytes_read = read(fd, buffer, size - 1);
    if (bytes_read <= 0) {
        buffer[0] = '\0';
        return -1;
    }
    buffer[bytes_read] = '\0';
    char *newline = strchr(buffer, '\n');
    if (newline) {
        *newline = '\0';
    }
    return bytes_read;
}

void apply_lock(int fd, int lock_type) {
    struct flock lock;
    lock.l_type = lock_type;
    lock.l_whence = SEEK_SET; 
    lock.l_start = 0;         
    lock.l_len = 0;    
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        safe_write(STDERR_FILENO, "Error: Failed to acquire file lock.\n");
        exit(1);
    }
}

struct User verifyLogin(int client_fd, struct User *curUserPtr) {
    int authState;
    read(client_fd, curUserPtr, sizeof(*curUserPtr));

    int fd = open(USERDETAILSFILE, O_RDWR);
    apply_lock(fd, F_WRLCK);

    struct User record = { .isActive = 1, .isLoggedIn = 0 };
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue; // Skip this record
        }
        // Compare the stored record with the user's input
        if (strcmp(curUserPtr->username, record.username) == 0 &&
            strcmp(curUserPtr->password, record.password) == 0) {
            if (!record.isActive){
                apply_lock(fd, F_UNLCK);
                close(fd);
                send(client_fd, &record, sizeof(struct User), 0);
                return record;
            }
            if (record.isLoggedIn){
                record.isLoggedIn = 2;
                apply_lock(fd, F_UNLCK);
                close(fd);
                send(client_fd, &record, sizeof(struct User), 0);
                return record;
            }
            record.isLoggedIn = 1;
            lseek(fd, -1*sizeof(record), SEEK_CUR);
            write(fd, &record, sizeof(record));
            *curUserPtr = record;
            break; // Stop reading once the user is found
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    send(client_fd, curUserPtr, sizeof(struct User), 0);
    return *curUserPtr;
}

struct User logout_server(int client_fd, struct User *curUserPtr){
    read(client_fd, curUserPtr, sizeof(struct User));
    printf("User %s chose to log off\n", curUserPtr->username);

    int fd = open(USERDETAILSFILE, O_RDWR);
    apply_lock(fd, F_WRLCK);
    struct User record;
    ssize_t bytes_read;
    int found = 0;

    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(curUserPtr->username, record.username) == 0) {
            record.isLoggedIn = 0;
            lseek(fd, -1*sizeof(record), SEEK_CUR);
            write(fd, &record, sizeof(record));
            *curUserPtr = record;
            found = 1;
            break;
        }
    }

    apply_lock(fd, F_UNLCK);
    close(fd);
    return *curUserPtr;
}

struct User getDetails_server(int client_fd, struct User *curUserPtr){
    read(client_fd, curUserPtr, sizeof(struct User));
    printf("User %s chose to view account details\n", curUserPtr->username);

    int fd = open(USERDETAILSFILE, O_RDONLY);
    apply_lock(fd, F_RDLCK);
    struct User record;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(curUserPtr->username, record.username) == 0) {
            *curUserPtr = record;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return *curUserPtr;
}