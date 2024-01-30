#ifndef _PARSECMD_H_
#define _PARSECMD_H_

#include <stdbool.h>

typedef struct {
    char *src_ip;
    char *src_volume_uuid;
    char *dest_ip;
    char *dest_volume_uuid;
    char *src_file_path;
    char *dest_file_path;
    char *input_file_path;
    char *file_path;
    char *tools_version;
    bool do_convert_zbs;
    bool do_convert_local;
    bool do_info;
} CommandLineArgs;

extern CommandLineArgs args;

int parse_args(int argc, char *argv[]);
#endif /* _PARSECMD_H_ */
