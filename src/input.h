#include <zos_vfs.h>

#define stdin DEV_STDIN

char *fgets(char *buf, int size, zos_dev_t dev);