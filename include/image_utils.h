#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <png.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct {
    int x, y;               
    int side_size;          
    int thickness;          
    int color_r, color_g, color_b;  
    bool fill;              
    int fill_r, fill_g, fill_b;     
} SquareOptions;


typedef struct {
    int width, height;      
    png_byte color_type;   
    png_byte bit_depth;     
    png_structp png_ptr;    
    png_infop info_ptr;     
    int number_of_passes;   
    png_bytep *row_pointers; 
} PngImage;


bool is_png(const char* filename);


void print_png_info(const char *filename);

bool read_png_file(const char *filename, PngImage *image);

bool write_png_file(const char* filename, PngImage* image);

void free_png(PngImage* image);

bool draw_square_with_diagonals(const char* input, const char* output, SquareOptions options);

bool apply_rgb_filter(const char* input, const char* output, const char* component, int value);

bool rotate_area(PngImage* image, int x1, int y1, int x2, int y2, int angle);

#endif 