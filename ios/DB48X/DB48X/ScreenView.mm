// ****************************************************************************
//  ScreenView.m                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Draw the DMCP screen
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#import "ScreenView.h"
#include "target.h"
#include "recorder.h"

#include "sim-dmcp.h"
#include <stdint.h>


// Copy of the pixel data
byte     pixelData[SIM_LCD_W * SIM_LCD_H * 4];
uint32_t lcd_copy[SIM_LCD_SCANLINE * SIM_LCD_H * color::BPP / 32];

RECORDER(screenview, 256, "Screen view image generation");


@implementation ScreenView
// ----------------------------------------------------------------------------
//   Data associated with a screen view
// ----------------------------------------------------------------------------
{
    uint refreshed;             // Number of times we rebuilt the image
}


- (id) init
// ----------------------------------------------------------------------------
//   Initialization
// ----------------------------------------------------------------------------
{
    self = [super init];
    if (self)
        refreshed = 0;
    return self;
}


- (unsigned) refreshCount
// ----------------------------------------------------------------------------
//   Return the number of refresh
// ----------------------------------------------------------------------------
{
    return refreshed;
}


-(UIImage *)imageFromLCD
// ----------------------------------------------------------------------------
//   Build an image from the DMCP LCD representation
// ----------------------------------------------------------------------------
//   This can (and should) run on the RPL thread
{
    record(screenview, "%u: imageFromLCD", refreshed);

    size_t bytesPerRow = SIM_LCD_W * 4;
    surface s(lcd_buffer, LCD_W, LCD_H, LCD_SCANLINE, LCD_W);
    pixword mask = ~(~0U << color::BPP);
    for (int y = 0; y < SIM_LCD_H; y++)
    {
        for (int xw = 0; xw < SIM_LCD_SCANLINE*color::BPP/32; xw++)
        {
            unsigned woffs = y * (SIM_LCD_SCANLINE*color::BPP/32) + xw;
            if (uint32_t diffs = lcd_copy[woffs] ^ lcd_buffer[woffs])
            {
                for (int bit = 0; bit < 32; bit += color::BPP)
                {
                    if ((diffs >> bit) & mask)
                    {
                        pixword bits = (lcd_buffer[woffs] >> bit) & mask;
                        color col(bits);
                        coord xx = (xw * 32 + bit) / color::BPP;
                        if (xx < SIM_LCD_W)
                        {
                            coord yy = y;
                            s.horizontal_adjust(xx, xx);
                            s.vertical_adjust(yy, yy);
                            uint pixoffs = yy * SIM_LCD_W + xx;
                            byte *pixel = &pixelData[pixoffs * 4];
#ifdef CONFIG_COLOR
                            pixel[0] = col.blue();
                            pixel[1] = col.green();
                            pixel[2] = col.red();
                            pixel[3] = 255;
#else
                            byte pixval = bits ? 220 : 0;
                            pixel[0] = pixel[1] = pixel[2] = pixel[3] = pixval;
#endif
                        }
                    }
                }
                lcd_copy[woffs] = lcd_buffer[woffs];
            }
        }
    }

    void *baseAddress = &pixelData;
    CGColorSpaceRef colorSpace  = CGColorSpaceCreateDeviceRGB();
    CGContextRef    context     = CGBitmapContextCreate(
        baseAddress,
        SIM_LCD_W,
        SIM_LCD_H,
        8,
        bytesPerRow,
        colorSpace,
        kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);

    CGImageRef cgImage = CGBitmapContextCreateImage(context);
    UIImage *image = [UIImage imageWithCGImage:cgImage];

    CGImageRelease(cgImage);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);

    record(screenview, "%u: imageFromLCD returns %p [%llu]",
           refreshed, (__bridge void *) image);

    refreshed++;

    return image;
}

@end
