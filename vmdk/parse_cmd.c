#include "parse_cmd.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

CommandLineArgs args = {
    .tools_version = "2147483647",  // default is 2^31-1 (unknown)
    .do_convert_zbs = false,
    .do_convert_local = false,
    .do_info = false,
    .dest_file_path = "dest.vmdk",
    .src_file_path = "",
    .dest_ip = "",
    .dest_volume_uuid = "",
    .src_ip = "",
    .src_volume_uuid = "",
};

void print_help() {
    printf("Usage:\n");
    printf("  vmdk-convert [options]\n");
    printf("Options:\n");
    printf("  -src_ip <ip>                 Specify the source ZBS Server address.\n");
    printf("  -src_volume_uuid <uuid>      Specify the source ZBS volume UUID.\n");
    printf("  -dest_ip <ip>                Specify the destination ZBS Server address.\n");
    printf("  -dest_volume_uuid <uuid>     Specify the destination ZBS volume UUID.\n");
    printf("  -src_file_path <path>        Specify the source file path.\n");
    printf("  -dest_file_path <path>       Specify the destination file path.\n");
    printf("  -i <file_path>               Displays information for specified virtual disk\n");
    printf("  -t <tools_version>           Converts source disk to destination disk with given tools version.\n");

    printf("  -h, --help                   Show this help message and exit.\n");
}

/* Check a string is number */
static bool isNumber(char *text) {
    int j;
    j = strlen(text);
    while (j--) {
        if (text[j] >= '0' && text[j] <= '9')
            continue;

        return false;
    }
    return true;
}

int parse_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return 1;
        } else if (strcmp(argv[i], "-src_ip") == 0 && i + 1 < argc) {
            args.src_ip = argv[++i];
        } else if (strcmp(argv[i], "-src_volume_uuid") == 0 && i + 1 < argc) {
            args.src_volume_uuid = argv[++i];
        } else if (strcmp(argv[i], "-dest_ip") == 0 && i + 1 < argc) {
            args.dest_ip = argv[++i];
        } else if (strcmp(argv[i], "-dest_volume_uuid") == 0 && i + 1 < argc) {
            args.dest_volume_uuid = argv[++i];
        } else if (strcmp(argv[i], "-src_file_path") == 0 && i + 1 < argc) {
            args.src_file_path = argv[++i];
        } else if (strcmp(argv[i], "-dest_file_path") == 0 && i + 1 < argc) {
            args.dest_file_path = argv[++i];
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            args.input_file_path = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            args.tools_version = argv[++i];
            if (!isNumber(args.tools_version)) {
                fprintf(stderr, "Invalid tools version: %s\n", args.tools_version);
                return 1;
            }
        } else {
            fprintf(stderr, "Error: Unrecognized argument %s\n", argv[i]);
            print_help();
            return 1;
        }
    }

    // 校验参数组合
    if (args.src_ip && args.src_volume_uuid && args.dest_ip && args.dest_volume_uuid) {
        args.do_convert_zbs = true;
    } else if (args.input_file_path && args.dest_file_path) {
        args.do_convert_local = true;
    } else if (args.input_file_path) {
        args.do_info = true;
    }

    if (!args.do_info && !args.do_convert_local && !args.do_convert_zbs) {
        print_help();
        return 1;
    }

    return 0;
}
