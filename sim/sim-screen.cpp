// ****************************************************************************
//  screen.cpp                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
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

#include "sim-screen.h"

#include "dmcp.h"
#include "recorder.h"
#include "sim-dmcp.h"
#include <target.h>


#if WASM

uintptr_t wasm_updated_screen = 0;

uintptr_t ui_lcd_buffer()
// ----------------------------------------------------------------------------
//   Recompute the pixmap
// ----------------------------------------------------------------------------
//   This should be done on the RPL thread to get a consistent picture
{
    uintptr_t result = wasm_updated_screen;
    wasm_updated_screen = 0;
    return result;
}



#else // Qt simulator

#include <QBitmap>
#include <QGraphicsPixmapItem>
#include <QTimer>

SimScreen *SimScreen::theScreen = nullptr;

// A copy of the LCD buffer
pixword lcd_copy[sizeof(lcd_buffer) / sizeof(*lcd_buffer)];

RECORDER(sim_help, 32, "Simulator help LCD pixmap sync");


SimScreen::SimScreen(QWidget *parent)
// ----------------------------------------------------------------------------
//   Initialize the screen
// ----------------------------------------------------------------------------
    : QGraphicsView(parent),
      screen_width(SIM_LCD_W),
      screen_height(SIM_LCD_H),
      scale(1),
#ifndef CONFIG_COLOR
      bgColor(230, 230, 230),
#else // CONFIG_COLOR
      bgColor(255, 255, 255),
#endif // CONFIG_COLOR
      fgColor(0, 0, 0),
      bgPen(bgColor),
      fgPen(fgColor),
      mainPixmap(SIM_LCD_W, SIM_LCD_H),
      pixmapWidth(SIM_LCD_W),
      contentXOffset(0),
      redraws(0)
{
    screen.clear();
    screen.setBackgroundBrush(QBrush(Qt::black));

    mainPixmap.fill(bgColor);
    mainScreen = screen.addPixmap(mainPixmap);
    mainScreen->setOffset(0.0, 0.0);
    mainScreen->setTransformationMode(Qt::SmoothTransformation);

    setScene(&screen);
    setSceneRect(0, -5, screen_width, screen_height + 5);
    centerOn(qreal(screen_width) / 2, qreal(screen_height) / 2);
    setScale(4.0);

    for (size_t i = 0; i < sizeof(lcd_copy) / sizeof(*lcd_copy); i++)
        lcd_copy[i] = ~lcd_buffer[i];

    show();

    theScreen = this;
}


SimScreen::~SimScreen()
// ----------------------------------------------------------------------------
//   SimScreen destructor
// ----------------------------------------------------------------------------
{
}


void SimScreen::setPixmapGeometry(int totalWidth, int xOffset)
// ----------------------------------------------------------------------------
//   Set the pixmap geometry for Android (wider pixmap with black bars)
// ----------------------------------------------------------------------------
{
    // Ensure pixmap is wide enough to contain LCD content and black bars
    int requiredWidth = xOffset + SIM_LCD_W;
    if (totalWidth < requiredWidth)
        totalWidth = requiredWidth;

    if (totalWidth != pixmapWidth || xOffset != contentXOffset)
    {
        pixmapWidth = totalWidth;
        contentXOffset = xOffset;

        // Create a new wider pixmap filled with black bars
        QPixmap newPixmap(pixmapWidth, SIM_LCD_H);
        newPixmap.fill(Qt::black);

        // Draw the LCD content area with the background color
        QPainter painter(&newPixmap);
        painter.fillRect(contentXOffset, 0, SIM_LCD_W, SIM_LCD_H, bgColor);
        painter.end();

        // Replace the pixmap
        mainPixmap = newPixmap;
        mainScreen->setPixmap(mainPixmap);

        // Update scene rect to match new pixmap width
        setSceneRect(0, 0, pixmapWidth, screen_height);
        centerOn(qreal(pixmapWidth) / 2, qreal(screen_height) / 2);

        // Force redraw of all LCD content
        for (size_t i = 0; i < sizeof(lcd_copy) / sizeof(*lcd_copy); i++)
            lcd_copy[i] = ~lcd_buffer[i];
    }
}


void SimScreen::setScale(qreal sf)
// ----------------------------------------------------------------------------
//   Adjust the scaling factor
// ----------------------------------------------------------------------------
{
    QGraphicsView::scale(sf / scale, sf / scale);
    scale = sf;

    QSize s;
    s.setWidth(0);
    s.setHeight((screen_height + 5) * scale);
    setMinimumSize(s);
}


void SimScreen::updatePixmap()
// ----------------------------------------------------------------------------
//   Recompute the pixmap
// ----------------------------------------------------------------------------
//   This should be done on the RPL thread to get a consistent picture
{
    // Monochrome screen
    QPainter pt(&mainPixmap);
#ifdef ANDROID
    // On Android, set clip rect to only allow drawing in the LCD content area
    pt.setClipRect(contentXOffset, 0, SIM_LCD_W, SIM_LCD_H);
#endif
    pixword mask = ~(~0U << color::BPP);
    surface s(lcd_buffer, LCD_W, LCD_H, LCD_SCANLINE, LCD_W);
    uint    drawn   = 0;
    uint    skipped = 0;
    uint    help_y  = 0;
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
#ifdef CONFIG_COLOR
                        QColor qcol(col.red(), col.green(), col.blue());
#else
                        QColor &qcol = bits ? bgColor : fgColor;
#endif
                        pt.setPen(qcol);

                        coord xx = (xw * 32 + bit) / color::BPP;
                        if (xx >= LCD_W)
                        {
                            if (y >= 23 && y < 217)
                                skipped++;
                            continue;
                        }
                        coord yy = y;
                        s.horizontal_adjust(xx, xx);
                        s.vertical_adjust(yy, yy);
                        if (y >= 23 && y < 217 && xx < 8)
                            help_y++;
                        drawn++;
#ifdef ANDROID
                        pt.drawPoint(xx + contentXOffset, yy);
#else
                        pt.drawPoint(xx, yy);
#endif
                    }
                }
                lcd_copy[woffs] = lcd_buffer[woffs];
            }
        }
    }
    if (skipped || help_y)
        record(sim_help, "pixmap drawn=%u skipped=%u help_left=%u",
               drawn, skipped, help_y);
    pt.end();
}


void SimScreen::refreshScreen()
// ----------------------------------------------------------------------------
//   Transfer the pixmap to the screen
// ----------------------------------------------------------------------------
//   This must be done on the main screen
{
    mainScreen->setPixmap(mainPixmap);
    QGraphicsView::update();
    redraws++;
}
#endif // WASM
