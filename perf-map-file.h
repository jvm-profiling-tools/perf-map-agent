FILE *perf_map_open(pid_t pid);
void perf_map_write_entry(FILE *method_file, const void* code_addr, unsigned int code_size, const char* entry);


