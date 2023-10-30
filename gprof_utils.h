#ifndef GPROF_UTILS_H
#define GPROF_UTILS_H

int get_address_list(int* address_list);

//in parameter
void map_address_to_name(char** names_list);

void report_data(int sampling_miss);

void process_data(int* buff);



#endif