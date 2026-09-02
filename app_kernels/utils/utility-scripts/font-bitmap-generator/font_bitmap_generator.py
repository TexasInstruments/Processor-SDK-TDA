#!/usr/bin/python3
#  Copyright (C) 2022 - 2026 Texas Instruments Incorporated - http://www.ti.com/
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""
A helper script to generate FONT Bitmap as C array.
Used in post_process library. More info on README.md
"""

import numpy as np
import cv2
from PIL import ImageFont, ImageDraw, Image
import math
import argparse
import os
import sys

MAX_BIT_LENGTH = 32

def generateBitmapAsArray(fontPath, fontSize, xPadding=0, preview=False, preview_time=100):
    font = ImageFont.truetype(fontPath, fontSize)
    fontName = os.path.basename(fontPath)
    ascent, descent = font.getmetrics()
    canvasHeight = ascent + descent
    canvasWidth = 0
    for i in range(94):
        character = chr(33+i)
        (width, baseline), (offset_x, offset_y) = font.font.getsize(character)
        canvasWidth = max(canvasWidth,width)

    if (canvasWidth % 2 != 0 ):
        canvasWidth += 1

    if (canvasHeight % 2 != 0 ):
        canvasHeight += 1

    canvasWidth += (2*xPadding)

    canvasWidth = int(canvasWidth)
    canvasHeight = int(canvasHeight)

    print(f"Generating textBlock of size {canvasWidth}x{canvasHeight}")
    fontName = fontName.split(".")[0]
    fontName = fontName.upper()
    variableName = f"uint{MAX_BIT_LENGTH}_t font_{fontName}_{canvasWidth}x{canvasHeight}[95][{math.ceil(canvasWidth*canvasHeight/MAX_BIT_LENGTH)}]"
    string = variableName + " = {\n"

    tabLength = len(string)

    for i in range(94):
        character = chr(33+i)
        canvas = np.zeros((canvasHeight,canvasWidth),dtype=np.uint8)
        img_pil = Image.fromarray(canvas)
        draw = ImageDraw.Draw(img_pil)
        w, h = font.getsize(character)
        draw.text((xPadding, 0), character, font=font, fill=255)
        arr = np.array(img_pil)
        arr[arr < 127] = 0
        arr[arr >= 127] = 1
        arr = arr.flatten()
        data = []
        bin_repr = ""
        for count,bit in enumerate(arr):
            bin_repr = str(bit)+bin_repr
            if ((count+1) % MAX_BIT_LENGTH == 0):
                data.append(hex(int(bin_repr, 2)))
                bin_repr = ""
        if len(bin_repr) > 0:
            last_byte = "0"*(MAX_BIT_LENGTH-len(bin_repr))
            last_byte += bin_repr
            data.append(hex(int(last_byte, 2)))
        string += " "*tabLength
        string += "{"
        for idx in range(len(data)-1):
            string += f"{data[idx]},"
        string += f"{data[-1]}"
        string += "},\n"
        if preview:
            cv2.imshow("Preview",np.array(img_pil))
            cv2.waitKey(preview_time)
    if preview:
        cv2.destroyAllWindows()
    
    string += " "*(tabLength-1)
    string += "};"
    return string

def main(sys_args):
    help_str = "Run : " + sys_args[0] + " -h for help"
    parser = argparse.ArgumentParser(usage=help_str, formatter_class=argparse.RawTextHelpFormatter)
    help_str_font = "Path to .ttf font file\n"
    parser.add_argument(
        "-f", "--font", help=help_str_font, type=str, required=True
    )
    help_str_font_sizes = "approximate lis of widths of font to generate in pixels\n"
    parser.add_argument(
        "-s", "--sizes", help=help_str_font_sizes, nargs="+", type=int, required=True
    )
    help_str_font_pad = "padding value in pixels (Default: 1)\n"
    parser.add_argument(
        "-p", "--padding", help=help_str_font_pad, type=int, default=2
    )
    help_str_preview = "Preview font in opencv window (Default: False)\n"
    parser.add_argument(
        "-v", "--vizualize", help=help_str_preview, action="store_true", default=False
    )
    
    args = parser.parse_args()
    if (not os.path.exists(args.font)):
    	print(f"[ERROR] [{args.font}] doesnt exist")
    data = ""
    fontSizes = sorted(list(set(args.sizes)))
    print(fontSizes)
    xPaddings = [args.padding] * len(fontSizes)
    for fontSize,xPadding in zip(fontSizes,xPaddings):
        data += generateBitmapAsArray(args.font,fontSize,xPadding=xPadding,preview=args.vizualize) + "\n\n"

    with open("data.txt","w+") as f:
        f.write(data)
    print("[SUCCESS] Font Bitmap Generated as C Array. Check data.txt")

if __name__ == "__main__":
    main(sys.argv)