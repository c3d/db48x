#ifndef SIM_SCREEN_H
#define SIM_SCREEN_H
// ****************************************************************************
//  screen.h                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Simulate the screen of the DM42
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

#if !WASM

#include <QGraphicsView>
#include <QGraphicsPixmapItem>

class SimScreen : public QGraphicsView
// ----------------------------------------------------------------------------
//    Screen emulation
// ----------------------------------------------------------------------------
{
    Q_OBJECT;

public:
    int                  screen_width;
    int                  screen_height;
    int                  keyboard_width;
    int                  keyboard_height;
    qreal                scale;

    QColor               bgColor;
    QColor               fgColor;
    QPen                 bgPen;
    QPen                 fgPen;

    QGraphicsScene       screen;
    QGraphicsPixmapItem *mainScreen;
    QPixmap              mainPixmap;

    uint                 redraws;

    static SimScreen    *theScreen;

  public:
    explicit SimScreen(QWidget *parent = 0);
    ~SimScreen();

  public:
    void        setScale(qreal _scale);
    void        updatePixmap();
    void        refreshScreen();
    static void update_pixmap() { theScreen->updatePixmap(); }
    static void refresh_lcd()   { theScreen->refreshScreen(); }
    static uint redraw_count()  { return theScreen->redraws; }
};

#endif // WASM

#endif // SIMSCREEN_H
