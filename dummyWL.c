#include "genutilsX.h"

int main() {
    int id = 1000;
    int fd = open(LOANSIDFILE, O_WRONLY);
    apply_lock(fd, F_WRLCK);
    write(fd, &id, sizeof(id));
    apply_lock(fd, F_UNLCK);
    close(fd);
    return 0;
}