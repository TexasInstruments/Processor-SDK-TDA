# font_bitmap_generator - A script to generate bitmap from font file and parse it as C array

This document walks through the tool font_bitmap_generator and how to use it
with post_process library.

# Installation

## Dependencies
This script has dependencies on the following

- opencv
- Pillow
- numpy

## Running the script

Sample run command:

python3 font_bitmap_generator.py \
-f \*path to ttf file\* \
-s \*list of width of each letter\* \
-p \*horizontal padding value [Default:1]\* \
-v \*enable preview on a opencv window\*

```
./font_bitmap_generator.py -f fonts/ANDALEMO.TTF -s 14 18 22 26
```

This will generate a double dimensional C arrays with bitmap as its values
and save it in data.txt file. The user can copy the required array and paste in
**app-apps-utils/src/app_nv12_font_utils.c** file and make necessary
changes in **getFont()** function to register the font generated.

The vairable name of C array is generated as follows

**uint32_t font_{font_name}_{width}x{height}**
