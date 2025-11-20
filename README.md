# 📌 PNG Image Processing Tool

A command-line PNG editor written in C using libpng.
Supports drawing geometric shapes, applying RGB filters, rotating selected areas, and viewing PNG metadata.

# Features

✔ Draw square borders

✔ Adjustable border thickness

✔ Optional filling of square interior

✔ Draw diagonals inside square

✔ RGB component filter (red, green, blue)

✔ Rotate image area (90°, 180°, 270°)

✔ View PNG metadata (resolution, color type, bit depth)

## Project Structure

/src
    image_utils.c
    main.c
/include
    image_utils.h
Makefile
README.md

## 🛠 Building using a MakeFile:
An executable file will appear after the build: ./png_editor

## Dependencies
- libpng
- zlib
- C compiler (gcc or clang)

## 🚀 Usage Examples

Rotate the area by 180°: 
png_editor --rotate --left_up 100.100 --right_down 300.300 --angle 180 -i in.png -o out.png

Filter by the green component:
png_editor --rgbfilter --component_name green --component_value 50 -i in.png -o green.png

Square without fill
png_editor --squared_lines --left_up 20.20 --side_size 120 --thickness 4 --color 0.0.255 -i in.png -o sq.png
