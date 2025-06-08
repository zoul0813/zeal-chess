#include <stdio.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_vfs.h>

// Reads a line from DEV_STDIN into buf, up to (size - 1) characters
char *fgets(char *buf, int size, zos_dev_t dev) {
    if (!buf || size <= 1) return NULL;

    uint16_t max_read = size - 1; // leave space for null terminator
    zos_err_t err = read(dev, buf, &max_read);
    if (err != ERR_SUCCESS) {
        puts("ERROR: read DEV_STDIN\n");
        return NULL;
    }

    // Null-terminate, truncate at newline if needed
    int i;
    for (i = 0; i < max_read; i++) {
        if (buf[i] == '\r' || buf[i] == '\n') {
            buf[i++] = '\n'; // normalize to \n
            break;
        }
    }
    buf[i] = '\0';
    return buf;
}