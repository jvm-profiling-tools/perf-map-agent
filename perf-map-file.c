#include <sys/types.h>
#include <stdio.h>

#include "perf-map-file.h"

FILE *perf_map_open(pid_t pid) {
    char filename[500];
    snprintf(filename, sizeof(filename), "/tmp/perf-%d.map", pid);
    return fopen(filename, "w");
}

void perf_map_write_entry(FILE *method_file, const void* code_addr, unsigned int code_size, const char* entry) {
    fprintf(method_file, "%lx %x %s\n", code_addr, code_size, entry);
    fflush(method_file);
}
