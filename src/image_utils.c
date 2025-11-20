#include "image_utils.h"
#include <png.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

bool is_png(const char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Couldnt open file\n");
        return false;
    }

    png_byte header[8];
    if (fread(header, 1, 8, fp) != 8) {
        fclose(fp);
        return false;
    }
    fclose(fp);

    return png_sig_cmp(header, 0, 8) == 0;
}

void print_png_info(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Couldnt open file\n");
        return;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    if (!png || !info) {
        fprintf(stderr, "Error initialization libpng\n");
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "Error open PNG\n");
        fclose(fp);
        png_destroy_read_struct(&png, &info, NULL);
        return;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    printf("PNG file information:\n");
    printf("Size: %d x %d pixels\n", width, height);
    printf("Color depth: %d bits\n", bit_depth);
    printf("Color type: ");
    switch (color_type) {
        case PNG_COLOR_TYPE_GRAY: puts("Grayscale"); break;
        case PNG_COLOR_TYPE_PALETTE: puts("Indexed-color"); break;
        case PNG_COLOR_TYPE_RGB: puts("RGB"); break;
        case PNG_COLOR_TYPE_RGBA: puts("RGBA"); break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: puts("Grayscale + Alpha"); break;
        default: puts("Неизвестно"); break;
    }

    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
}

bool read_png_file(const char* filename, PngImage* image) {
    unsigned char header[8];

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return false;
    }

    if (fread(header, 1, 8, fp) != 8) {
        fclose(fp);
        return false;
    }

    if (png_sig_cmp(header, 0, 8)) {
        fclose(fp);
        return false;
    }

    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!image->png_ptr) {
        fclose(fp);
        return false;
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr) {
        png_destroy_read_struct(&image->png_ptr, NULL, NULL);
        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(image->png_ptr))) {
        png_destroy_read_struct(&image->png_ptr, &image->info_ptr, NULL);
        fclose(fp);
        return false;
    }

    png_init_io(image->png_ptr, fp);
    png_set_sig_bytes(image->png_ptr, 8);

    png_read_info(image->png_ptr, image->info_ptr);

    image->width = png_get_image_width(image->png_ptr, image->info_ptr);
    image->height = png_get_image_height(image->png_ptr, image->info_ptr);
    image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
    image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);
    image->number_of_passes = png_set_interlace_handling(image->png_ptr);

    if (image->bit_depth == 16) {
        png_set_strip_16(image->png_ptr);
    }

    if (image->color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(image->png_ptr);
    }
    
    if (image->color_type == PNG_COLOR_TYPE_GRAY && image->bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(image->png_ptr);
    }

    if (png_get_valid(image->png_ptr, image->info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(image->png_ptr);
    }

    if (image->color_type == PNG_COLOR_TYPE_RGB || image->color_type == PNG_COLOR_TYPE_GRAY || image->color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(image->png_ptr, 0xFF, PNG_FILLER_AFTER); 
    }

    if (image->color_type == PNG_COLOR_TYPE_GRAY || image->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(image->png_ptr); 
    }

    png_read_update_info(image->png_ptr, image->info_ptr);

    image->row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * image->height);
    if (!image->row_pointers) {
        png_destroy_read_struct(&image->png_ptr, &image->info_ptr, NULL);
        fclose(fp);
        return false;
    }

    for (int y = 0; y < image->height; y++) {
        image->row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));
        if (!image->row_pointers[y]) {
            for (int i = 0; i < y; i++) {
                free(image->row_pointers[i]);
            }
            free(image->row_pointers);
            png_destroy_read_struct(&image->png_ptr, &image->info_ptr, NULL);
            fclose(fp);
            return false;
        }
    }

    png_read_image(image->png_ptr, image->row_pointers);
    fclose(fp);
    return true;
}

bool write_png_file(const char* filename, PngImage* image) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        return false;
    }

    image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!image->png_ptr) {
        fclose(fp);
        return false;
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr) {
        png_destroy_write_struct(&image->png_ptr, NULL);
        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(image->png_ptr))) {
        png_destroy_write_struct(&image->png_ptr, &image->info_ptr);
        fclose(fp);
        return false;
    }

    png_init_io(image->png_ptr, fp);

    png_set_IHDR(
        image->png_ptr,
        image->info_ptr,
        image->width,
        image->height,
        8, 
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(image->png_ptr, image->info_ptr);
    png_write_image(image->png_ptr, image->row_pointers);
    png_write_end(image->png_ptr, NULL);

    png_destroy_write_struct(&image->png_ptr, &image->info_ptr);
    fclose(fp);
    return true;
}

void free_png(PngImage* image) {
    if (image->row_pointers) {
        for (int y = 0; y < image->height; y++) {
            free(image->row_pointers[y]);
        }
        free(image->row_pointers);
        image->row_pointers = NULL;
    }
}

bool is_in_main_diagonal(int x, int y, SquareOptions options) {
    int dx = x - options.x;
    int dy = y - options.y;
    return abs(dx - dy) <= (options.thickness / 2);
}

bool is_in_anti_diagonal(int x, int y, SquareOptions options) {
    int dx = x - options.x;
    int dy = y - options.y;
    return abs(dx + dy - (options.side_size - 1)) <= (options.thickness / 2);
}

int clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

bool draw_square_with_diagonals(const char* input, const char* output, SquareOptions options) {
    if (!is_png(input)) return false;

    PngImage image;
    if (!read_png_file(input, &image)) return false;

    int half = options.thickness / 2;

    int draw_x0 = options.x - half;
    int draw_y0 = options.y - half;

    int draw_x1 = draw_x0 + options.side_size + options.thickness;
    int draw_y1 = draw_y0 + options.side_size + options.thickness;

    int x_start = clamp(draw_x0, 0, image.width);
    int y_start = clamp(draw_y0, 0, image.height);
    int x_end   = clamp(draw_x1, 0, image.width);
    int y_end   = clamp(draw_y1, 0, image.height);

    for (int line = 0; line < options.thickness; line++) {
        int y_top    = draw_y0 + line;
        int y_bottom = draw_y1 - 1 - line;

        if (y_top >= y_start && y_top < y_end) {
            png_bytep row = image.row_pointers[y_top];
            for (int x = x_start; x < x_end; x++) {
                png_bytep px = &row[x * 4];
                px[0] = options.color_r;
                px[1] = options.color_g;
                px[2] = options.color_b;
                px[3] = 255;
            }
        }
        if (y_bottom >= y_start && y_bottom < y_end && y_bottom != y_top) {
            png_bytep row = image.row_pointers[y_bottom];
            for (int x = x_start; x < x_end; x++) {
                png_bytep px = &row[x * 4];
                px[0] = options.color_r;
                px[1] = options.color_g;
                px[2] = options.color_b;
                px[3] = 255;
            }
        }
    }

    for (int line = 0; line < options.thickness; line++) {
        int x_left  = draw_x0 + line;
        int x_right = draw_x1 - 1 - line;

        for (int y = y_start; y < y_end; y++) {
            png_bytep row = image.row_pointers[y];
            if (x_left >= x_start && x_left < x_end) {
                png_bytep px = &row[x_left * 4];
                px[0] = options.color_r;
                px[1] = options.color_g;
                px[2] = options.color_b;
                px[3] = 255;
            }
            if (x_right >= x_start && x_right < x_end && x_right != x_left) {
                png_bytep px = &row[x_right * 4];
                px[0] = options.color_r;
                px[1] = options.color_g;
                px[2] = options.color_b;
                px[3] = 255;
            }
        }
    }

    if (options.fill) {
        int fill_x0 = draw_x0 + options.thickness;
        int fill_x1 = draw_x1 - options.thickness;
        int fill_y0 = draw_y0 + options.thickness;
        int fill_y1 = draw_y1 - options.thickness;

        for (int y = fill_y0; y < fill_y1; y++) {
            if (y < 0 || y >= image.height) continue;
            png_bytep row = image.row_pointers[y];
            for (int x = fill_x0; x < fill_x1; x++) {
                if (x < 0 || x >= image.width) continue;
                png_bytep px = &row[x * 4];
                px[0] = options.fill_r;
                px[1] = options.fill_g;
                px[2] = options.fill_b;
                px[3] = 255;
            }
        }
    }
    for (int y = y_start; y < y_end; y++) {
        png_bytep row = image.row_pointers[y];
        for (int x = x_start; x < x_end; x++) {
            bool draw_main = is_in_main_diagonal(x, y, options);
            bool draw_anti = is_in_anti_diagonal(x, y, options);
            
            if (draw_main || draw_anti) {
                png_bytep px = &row[x * 4];
                px[0] = options.color_r;
                px[1] = options.color_g;
                px[2] = options.color_b;
                px[3] = 255;
            }
        }
    }


    if (!write_png_file(output, &image)) {
        free_png(&image);
        return false;
    }
    free_png(&image);
    return true;
}
        
bool apply_rgb_filter(const char* input, const char* output, const char* component, int value) {
    if (!is_png(input)) {
        return false;
    }

    PngImage image;
    if (!read_png_file(input, &image)) return false;

    int component_index = -1;
    if (strcmp(component, "red") == 0) component_index = 0;
    else if (strcmp(component, "green") == 0) component_index = 1;
    else if (strcmp(component, "blue") == 0) component_index = 2;
    else {
        free_png(&image);
        return false;
    }

    if (value < 0 || value > 255) {
        free_png(&image);
        return false;
    }

    for (int y = 0; y < image.height; y++) {
        png_bytep row = image.row_pointers[y];
        for (int x = 0; x < image.width; x++) {
            png_bytep px = &(row[x * 4]);

            if (px[3] == 0) continue; 

            if (px[component_index] != value) {
                px[component_index] = (png_byte)value;
            }
        }
    }

    if (!write_png_file(output, &image)) {
        free_png(&image);
        return false;
        }
    free_png(&image);
    return true;
}

bool rotate_area(PngImage *image, int x1, int y1, int x2, int y2, int angle) {
    if (angle != 90 && angle != 180 && angle != 270) {
        return false;
    }
    if (x1 > x2) { int tmp = x1; x1 = x2; x2 = tmp; }
    if (y1 > y2) { int tmp = y1; y1 = y2; y2 = tmp; }

    int width = x2 - x1;
    int height = y2 - y1;

    if (width <= 0 || height <= 0) {
        return false;
    }
    png_bytep *temp = malloc(height * sizeof(png_bytep));
    for (int i = 0; i < height; i++) {
        temp[i] = malloc(4 * width);
        for (int j = 0; j < width; j++) {
            int src_x = x1 + j;
            int src_y = y1 + i;
            if (src_x >= 0 && src_x < image->width && 
                src_y >= 0 && src_y < image->height) {
                memcpy(&temp[i][j * 4], &image->row_pointers[src_y][src_x * 4], 4);
            } else {
                memset(&temp[i][j * 4], 0, 4);
            }
        }
    }

    int rot_width = (angle == 180) ? width : height;
    int rot_height = (angle == 180) ? height : width;

    png_bytep *rotated = malloc(rot_height * sizeof(png_bytep));
    for (int i = 0; i < rot_height; i++) {
        rotated[i] = malloc(4 * rot_width);
        memset(rotated[i], 0, 4 * rot_width);
    }
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int new_x, new_y;
            switch (angle) {
                case 90:
                    new_x = y;
                    new_y = rot_height - 1 - x;
                    break;
                case 180:
                    new_x = width - 1 - x;
                    new_y = height - 1 - y;
                    break;
                case 270:
                    new_x = height - 1 - y;
                    new_y = x;
                    break;
                default:
                    new_x = x;
                    new_y = y;
            }
            memcpy(&rotated[new_y][new_x * 4], &temp[y][x * 4], 4);
        }
    }
    float center_x = x1 + (width - 1) / 2.0f;
    float center_y = y1 + (height - 1) / 2.0f;

    int start_x = (int)roundf(center_x - (rot_width - 1) / 2.0f);
    int start_y = (int)roundf(center_y - (rot_height - 1) / 2.0f);  

    for (int y = 0; y < rot_height; y++) {
        int img_y = start_y + y;
        if (img_y < 0 || img_y >= image->height) continue;
        png_bytep row = image->row_pointers[img_y];
        for (int x = 0; x < rot_width; x++) {
            int img_x = start_x + x;
            if (img_x >= 0 && img_x < image->width) {
                memcpy(&row[img_x * 4], &rotated[y][x * 4], 4);
            }
        }
    }
    for (int i = 0; i < height; i++) free(temp[i]);
    for (int i = 0; i < rot_height; i++) free(rotated[i]);
    free(temp);
    free(rotated);

    return true;
}