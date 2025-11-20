#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "image_utils.h"

void print_help() {
    printf("Course work for option 4.24, created by Karina Kim.\n");
    printf("\nUsage: png_editor [options] input.png\n");
    printf("Options:\n");
    printf("  -h, --help                      Show this help message\n");
    printf("      --info                      Print PNG file info\n");
    printf("  -i, --input FILE                Specify input file name\n");
    printf("  -o, --output FILE               Specify output file name (default: out.png)\n");
    printf("      --squared_lines             Draw square with diagonals\n");
    printf("      --left_up X.Y               Top-left corner of square or rotation area\n");
    printf("      --side_size N               Side size of the square\n");
    printf("      --thickness N               Line thickness\n");
    printf("      --color R.G.B               Border color\n");
    printf("      --fill                      Fill the square\n");
    printf("      --fill_color R.G.B          Fill color\n");
    printf("      --rgbfilter                 Apply RGB filter\n");
    printf("      --component_name NAME       Component to change (red, green, blue)\n");
    printf("      --component_value VAL       Value to set (0-255)\n");
    printf("      --rotate                    Rotate part of image\n");
    printf("      --right_down X.Y            Bottom-right corner of rotation area\n");
    printf("      --angle 90|180|270          Rotation angle\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_help();
        return 0;
    }

    char *input_file = NULL;
    char *output_file = "out.png";
    int do_info = 0, do_square = 0, do_rgbfilter = 0, do_rotate = 0;
    SquareOptions sq_opts = {0};
    char *component_name = NULL;
    int component_value = -1;
    int left_up_x = -1, left_up_y = -1, right_down_x = -1, right_down_y = -1, angle = -1;

    static struct option long_options[] = {
        {"help",            no_argument,       NULL, 'h'},
        {"input",           required_argument, NULL, 'i'},
        {"output",          required_argument, NULL, 'o'},
        {"squared_lines",   no_argument,       NULL, 1000},
        {"left_up",         required_argument, NULL, 1001},
        {"right_down",      required_argument, NULL, 1002},
        {"side_size",       required_argument, NULL, 1003},
        {"thickness",       required_argument, NULL, 1004},
        {"color",           required_argument, NULL, 1005},
        {"fill",            no_argument,       NULL, 1006},
        {"fill_color",      required_argument, NULL, 1007},
        {"rgbfilter",       no_argument,       NULL, 1008},
        {"component_name",  required_argument, NULL, 1009},
        {"component_value", required_argument, NULL, 1010},
        {"rotate",          no_argument,       NULL, 1011},
        {"angle",           required_argument, NULL, 1012},
        {"info",            no_argument,       NULL, 1013},
        {0, 0, 0, 0}
    };

    int opt, long_index = 0;
    while ((opt = getopt_long(argc, argv, "hio:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                print_help();
                return 0;

            case 'i':  
                input_file = optarg;
                break;

            case 'o':
                output_file = optarg;
                break;

            case 1000:
                do_square = 1;
                break;

            case 1001:
                if (sscanf(optarg, "%d.%d", &sq_opts.x, &sq_opts.y) != 2) {
                    fprintf(stderr, "Error: Invalid format for --left_up, expected format: X.Y\n");
                    return 40;
                }
                left_up_x = sq_opts.x;
                left_up_y = sq_opts.y;
                break;

            case 1002:
                if (sscanf(optarg, "%d.%d", &right_down_x, &right_down_y) != 2) {
                    fprintf(stderr, "Error: Invalid format for --right_down, expected format: X.Y\n");
                    return 40;
                }
                break;

            case 1003:
                sq_opts.side_size = atoi(optarg);
                if (sq_opts.side_size <= 0) {
                    fprintf(stderr, "Error: Invalid side size for square, must be a positive integer\n");
                    return 41;
                }
                break;

            case 1004:
                sq_opts.thickness = atoi(optarg);
                if (sq_opts.thickness <= 0) {
                    fprintf(stderr, "Error: Invalid thickness, must be a positive integer\n");
                    return 41;
                }
                break;

            case 1005:
                if (sscanf(optarg, "%d.%d.%d", &sq_opts.color_r, &sq_opts.color_g, &sq_opts.color_b) != 3) {
                    fprintf(stderr, "Error: Invalid format for --color, expected format: R.G.B\n");
                    return 40;
                }
                break;

            case 1006:
                sq_opts.fill = 1;
                break;

            case 1007:
                if (sscanf(optarg, "%d.%d.%d", &sq_opts.fill_r, &sq_opts.fill_g, &sq_opts.fill_b) != 3) {
                    fprintf(stderr, "Error: Invalid format for --fill_color, expected format: R.G.B\n");
                    return 40;
                }
                break;

            case 1008:
                do_rgbfilter = 1;
                break;

            case 1009:
                component_name = optarg;
                break;

            case 1010:
                component_value = atoi(optarg);
                if (component_value < 0 || component_value > 255) {
                    fprintf(stderr, "Error: Invalid value for component. It must be between 0 and 255.\n");
                    return 41;
                }
                break;

            case 1011:
                do_rotate = 1;
                break;

            case 1012:
                angle = atoi(optarg);
                if (angle % 90 != 0) {
                    fprintf(stderr, "Error: Invalid angle for --angle. It must be 90, 180, or 270.\n");
                    return 40;
                }
                break;

            case 1013:
                do_info = 1;
                break;

            default:
                fprintf(stderr, "Unknown option. Use --help\n");
                return 42;
        }
    }

    if (!input_file) {
        if (optind < argc) {
            input_file = argv[optind];
        } else {
            fprintf(stderr, "Error: Missing input PNG file.\n");
            return 43;
        }
    }

    if (strcmp(input_file, output_file) == 0) {
        fprintf(stderr, "Error: Input and output files must be different.\n");
        return 44;
    }

    if (do_info) {
        print_png_info(input_file);
        return 0;
    }

    int actions = do_square + do_rgbfilter + do_rotate;
    if (actions != 1) {
        fprintf(stderr, "Error: only one action can be performed.\n");
        return 45;
    }

    if (do_square) {
        if (sq_opts.side_size <= 0 || sq_opts.thickness <= 0) {
            fprintf(stderr, "Error: side_size and thickness must be > 0.\n");
            return 45;
        }
        if (!is_png(input_file)) {
            fprintf(stderr, "Error: %s is not a PNG file\n", input_file);
            return 46;
        }
        draw_square_with_diagonals(input_file, output_file, sq_opts);
    } else if (do_rgbfilter) {
        if (!component_name || component_value < 0 || component_value > 255) {
            fprintf(stderr, "Error: specify --component_name and --component_value (0-255).\n");
            return 41;
        }
        if (strcmp(component_name, "red") != 0 && 
            strcmp(component_name, "green") != 0 && 
            strcmp(component_name, "blue") != 0) {
            fprintf(stderr, "Error: component must be red, green or blue\n");
            return 47;
        }
        
        if (!is_png(input_file)) {
            fprintf(stderr, "Error: %s is not a PNG file\n", input_file);
            return 46;
        }
        apply_rgb_filter(input_file, output_file, component_name, component_value);
    } else if (do_rotate) {
        if (left_up_x == -1 || right_down_x == -1 || angle == -1) {
            fprintf(stderr, "Error: --left_up, --right_down, and --angle are required for rotation.\n");
            return 40;
        }
        if (angle != 90 && angle != 180 && angle != 270) {
            fprintf(stderr, "Error: angle must be 90, 180, or 270.\n");
            return 41;
        }
        if (!is_png(input_file)) {
            fprintf(stderr, "Error: %s is not a PNG file\n", input_file);
            return 46;
        }
        if (left_up_x >= right_down_x || left_up_y >= right_down_y) {
            fprintf(stderr, "Error: invalid area coordinates\n");
            return 48;
        }
        PngImage image;
        if (!read_png_file(input_file, &image)) {
            fprintf(stderr, "Error reading PNG file\n");
            return 49;
        }
        rotate_area(&image, left_up_x, left_up_y, right_down_x, right_down_y, angle);
        write_png_file(output_file, &image);
        free_png(&image);
    }

    return 0;
}
