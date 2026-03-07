// ****************************************************************************
//  sim-window.cpp                                                DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Main window for the DM42 simulator
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

#include "sim-window.h"

#include "dmcp.h"
#include "main.h"
#include "recorder.h"
#include "sim-dmcp.h"
#include "symbol.h"
#include "sysmenu.h"
#include "target.h"
#include "tests.h"
#include "user_interface.h"

#if WASM
#include "emcc.h"
#else
#include "sim-rpl.h"
#include "ui_sim-window.h"
#include <iostream>
#include <QAudioFormat>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDevice>
#include <QAudioSink>
#include <QMediaDevices>
#else
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#endif
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMessageBox>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QtCore>
#include <QtGui>
#include <QtMath>
#ifdef ANDROID
#include <QDir>
#include <QSettings>
#include <atomic>
#include "version.h"

void extract_android_assets();

#endif // ANDROID
#endif // WASM


RECORDER(sim_window, 16, "Window management for the simulator");
RECORDER(sim_keys, 16, "Keys from the simulator");
RECORDER(sim_audio, 16, "Audio for the simulator");

extern bool run_tests;
extern bool shift_held;
extern bool alt_held;

#if !WASM

MainWindow *MainWindow::mainWindow = nullptr;
qreal MainWindow::userScaling = 1.0;

MainWindow::MainWindow(QWidget *parent)
// ----------------------------------------------------------------------------
//    The main window of the simulator
// ----------------------------------------------------------------------------
    : QMainWindow(parent), ui(), rpl(this), tests(this), highlight(),
      keyboard_width(698), keyboard_height(878),
      resizeDirection(0),
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      devices(new QMediaDevices(this)),
#endif
      audio(), generator()
{
    mainWindow = this;

    QCoreApplication::setOrganizationName("DB48X");
    QCoreApplication::setApplicationName(PROGRAM_NAME);

    ui.setupUi(this);

    // Disable automatic layout management for manual control
    QLayout *pageLayout = ui.stackedWidget->widget(0)->layout();
    if (pageLayout)
    {
        pageLayout->setParent(nullptr);
        delete pageLayout;
    }

    ui.centralWidget->layout()->setContentsMargins(0, 0, 0, 0);
    ui.centralWidget->layout()->setSpacing(0);

    ui.keyboard->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.keyboard->installEventFilter(this);
    ui.screen->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.screen->installEventFilter(this);
    ui.keyboard->setStyleSheet("border-image: "
                               "url(:/bitmap/keymap.png) "
                               "0 0 0 0 stretch stretch;");

    highlight = new Highlight(ui.keyboard);
    highlight->setGeometry(0,0,0,0);
    highlight->show();

    setWindowTitle(PROGRAM_NAME);

    QObject::connect(this, SIGNAL(keyResizeSignal(const QRect &)),
                     highlight, SLOT(keyResizeSlot(const QRect &)));

#ifdef ANDROID
    // On Android, center the stacked widget within the central widget
    ui.centralWidget->setStyleSheet("background-color: black;");

    ui.stackedWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // Set the layout to center its content horizontally
    QLayout *layout = ui.centralWidget->layout();
    if (layout)
        layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    layout = this->layout();
    if (layout)
        layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    adjustSize();

    QSize osz = size();
    QSize isz = ui.centralWidget->size();
    int x = (osz.width() - isz.width()) / 2;
    int y = (osz.height() - isz.height()) / 2;
    ui.centralWidget->move(x, y);

#else
    if (userScaling != 1.0)
    {
        QSize sz = size();
        resize(sz.width() * userScaling, sz.height() * userScaling);
    }
#endif

    // Audio setup
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(devices, &QMediaDevices::audioOutputsChanged,
            this, &MainWindow::updateAudioDevices);
    initializeAudio(devices->defaultAudioOutput(), 0);
#else
    initializeAudio(QAudioDeviceInfo::defaultOutputDevice(), 0);
#endif

    setlocale(LC_ALL, "C");

    // Set initial geometry manually since we disabled layout management
    QResizeEvent initialResize(size(), size());
    resizeEvent(&initialResize);

#ifdef ANDROID
    extract_android_assets();

    connect(qGuiApp, &QGuiApplication::applicationStateChanged,
            this, &MainWindow::handleAppStateChange);
#endif

    rpl.start();
    if (run_tests)
    {
        ui_ms_sleep(1000);      // In case we are loading a file
        tests.start();
    }
}


MainWindow::~MainWindow()
// ----------------------------------------------------------------------------
//  Destroy the main window
// ----------------------------------------------------------------------------
{
    key_push(tests::EXIT_PGM);
    record(sim_audio, "Deleting audio");
}


void MainWindow::resizeEvent(QResizeEvent * event)
// ----------------------------------------------------------------------------
//   Resizing the window
// ----------------------------------------------------------------------------
{
    // Compute size adjustment
    QSize  oldSize = event->oldSize();
    QSize  delta   = (oldSize.width() >= 0 && oldSize.height() >= 0)
                       ? event->size() - oldSize
                       : QSize(0, 0);
    int    dw      = delta.width();
    int    dh      = delta.height();

    // Compute new size
    QSize  newSize = size();
    QPoint newPos  = pos();
    int    nx      = newPos.x();
    int    ny      = newPos.y();
    int    nw      = newSize.width();
    int    nh      = newSize.height();

    record(sim_window,
           "Resize   x=%d y=%d w=%d h=%d dw=%d dh=%d",
           nx, ny, nw, nh, dw, dh);

    // Preserve aspect ratio (420:800)
    const qreal r      = 420.0 / 800.0;
    bool        resize = false;
    if (nw > nh * r * 1.01)
    {
        if (!resizeDirection)
            resizeDirection = 2 - (dw > 0 || dh > 0);
        resize = true;
        record(sim_window, "Wide     x=%d y=%d w=%d h=%d dw=%d dh=%d",
               nx, ny, nw, nh, dw, dh);
    }
    else if (nw < nh * r * 0.99)
    {
        if (!resizeDirection)
            resizeDirection = 1 + (dh > 0 || dw > 0);
        resize = true;
        record(sim_window, "Narrow   x=%d y=%d w=%d h=%d dw=%d dh=%d",
               nx, ny, nw, nh, dw, dh);
    }

    if (resize && resizeDirection)
    {
        if (resizeDirection == 1)
            nh = nw / r;
        else if (resizeDirection == 2)
            nw = nh * r;
        record(sim_window, "Resizing dir=%d w=%d h=%d",
               resizeDirection, nw, nh);
#ifndef ANDROID
        QMainWindow::resize(nw, nh);
#endif
    }

    int   sw = ui.screen->screen_width;
    int   sh = ui.screen->screen_height + 5;

    // Screen uses full window width
    qreal sr = qreal(nw) / sw;

    // Calculate scaled screen dimensions
    qreal scaledSW = nw;  // Full width
    qreal scaledSH = sh * sr;

    // Remaining space for the keyboard
    qreal kh = nh - scaledSH;

    // Scale keyboard proportionally to fill remaining height
    qreal kr = kh / keyboard_height;
    qreal kw = keyboard_width * kr;

    // If keyboard is too wide, scale it down to window width
    if (kw > nw)
    {
        kr = qreal(nw) / keyboard_width;
        kw = nw;
        kh = keyboard_height * kr;
    }

    // Set screen ratio and geometry
    int xOffset      = (nw - scaledSW) / 2;
    int yOffset      = (nh - scaledSH - kh) / 2;
    int screenWidth  = scaledSW;
    int screenHeight = scaledSH;

#ifdef ANDROID
    // On Android: use a wider pixmap with black bars and position at x=0
    // This is because the primary screen needs to be at x=0, otherwise
    // we observe really weird refresh delays on the leftmost columns
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen)
    {
        QRect availableGeometry = screen->availableGeometry();
        int availableWidth = availableGeometry.width();
        xOffset = (availableWidth - nw) / 2;

        // Calculate where LCD content should be drawn within the pixmap
        int unscaledPixmapWidth = availableWidth / sr;
        int unscaledContentX = (xOffset + (nw - scaledSW) / 2) / sr;

        // Ensure pixmap is wide enough
        int requiredPixmapWidth = unscaledContentX + ui.screen->screen_width;
        if (unscaledPixmapWidth < requiredPixmapWidth)
            unscaledPixmapWidth = requiredPixmapWidth;

        // Set the pixmap to span the full width with black bars
        ui.screen->setPixmapGeometry(unscaledPixmapWidth, unscaledContentX);
        screenWidth = unscaledPixmapWidth * sr;
        xOffset = 0;
        nw = availableWidth;
    }
#endif // ANDROID

    // Position the screen view (always at x=0 on Android)
    QRect sframe(xOffset, yOffset, screenWidth, screenHeight);
    ui.screen->setGeometry(sframe);
    ui.screen->setScale(sr);

    QRect kframe((nw - kw) / 2, yOffset + screenHeight, kw, kh);
    ui.keyboard->setGeometry(kframe);
}


#ifdef ANDROID
void extract_android_assets()
{
    QString sandboxDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(sandboxDir);

    QSettings settings("DB48X", "Emulator");
    QString currentAssetVersion = DB48X_VERSION;
    QString savedAssetVersion = settings.value("AssetVersion", "").toString();

    if (savedAssetVersion != currentAssetVersion) {
        QStringList filesToExtract = {"db48x.idx", "db48x.md"};

        for (const QString& fileName : filesToExtract) {
            QString assetPath = ":/help/" + fileName; // Check your Qt resource prefix
            QString targetPath = sandboxDir + "/help/" + fileName;

            if (QFile::exists(targetPath)) {
                QFile::remove(targetPath);
            }

	    // Create the directory structure if it doesn't exist
	    QFileInfo targetInfo(targetPath);
	    QDir().mkpath(targetInfo.absolutePath());

            QFile assetFile(assetPath);
            if (assetFile.copy(targetPath)) {
                QFile::setPermissions(targetPath,
                    QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser);
            }
        }

	settings.setValue("AssetVersion", currentAssetVersion);
    }

    QDir::setCurrent(sandboxDir);
}


static std::atomic<bool> is_dialog_open{false};

void MainWindow::handleAppStateChange(Qt::ApplicationState state)
// ----------------------------------------------------------------------------
//   Trigger background auto-save when Android suspends the app
// ----------------------------------------------------------------------------
{
    // If a native dialog is currently open, the user is actively managing state.
    // Abort the background auto-save to prevent recursive Intents.
    if (is_dialog_open)
        return;

    static bool isSaved = false;

    if (state == Qt::ApplicationActive) {
        isSaved = false;
    }
    else if ((state == Qt::ApplicationSuspended || state == Qt::ApplicationHidden)
             && !isSaved) // Check both suspend and hidden + avoid double save
    {
        // Call the core DB48X save function directly
        // (This function is defined in sysmenu.cc)
        extern bool save_system_state_silent();
        save_system_state_silent();

        record(sim_window, "Android auto-save triggered");

	isSaved = true;
    }
}
#endif


const int keyMap[] =
// ----------------------------------------------------------------------------
//   Key map for the DM42
// ----------------------------------------------------------------------------
{
    // Actual key mappings are in the relevant platform's target.h
    Qt::Key_Tab,        KB_ALPHA,
    Qt::Key_SysReq,     KB_ON,
    Qt::Key_Escape,     KB_ESC,
    Qt::Key_Period,     KB_DOT,
    Qt::Key_Space,      KB_SPC,
    Qt::Key_Question,   KB_QUESTION,
    Qt::Key_Meta,       KB_SHIFT,

    Qt::Key_Plus,       KB_ADD,
    Qt::Key_Minus,      KB_SUB,
    Qt::Key_Asterisk,   KB_MUL,
    Qt::Key_Slash,      KB_DIV,

    Qt::Key_Enter,      KB_ENT,
    Qt::Key_Return,     KB_ENT,
    Qt::Key_Backspace,  KB_BKS,
    Qt::Key_Up,         KB_UP,
    Qt::Key_Down,       KB_DN,
    Qt::Key_Left,       KB_LF,
    Qt::Key_Right,      KB_RT,

    Qt::Key_F1,         KB_F1,
    Qt::Key_F2,         KB_F2,
    Qt::Key_F3,         KB_F3,
    Qt::Key_F4,         KB_F4,
    Qt::Key_F5,         KB_F5,
    Qt::Key_F6,         KB_F6,

    Qt::Key_F8,         KEY_SCREENSHOT,

    Qt::Key_0,          KB_0,
    Qt::Key_1,          KB_1,
    Qt::Key_2,          KB_2,
    Qt::Key_3,          KB_3,
    Qt::Key_4,          KB_4,
    Qt::Key_5,          KB_5,
    Qt::Key_6,          KB_6,
    Qt::Key_7,          KB_7,
    Qt::Key_8,          KB_8,
    Qt::Key_9,          KB_9,
    Qt::Key_A,          KB_A,
    Qt::Key_B,          KB_B,
    Qt::Key_C,          KB_C,
    Qt::Key_D,          KB_D,
    Qt::Key_E,          KB_E,
    Qt::Key_F,          KB_F,
    Qt::Key_G,          KB_G,
    Qt::Key_H,          KB_H,
    Qt::Key_I,          KB_I,
    Qt::Key_J,          KB_J,
    Qt::Key_K,          KB_K,
    Qt::Key_L,          KB_L,
    Qt::Key_M,          KB_M,
    Qt::Key_N,          KB_N,
    Qt::Key_O,          KB_O,
    Qt::Key_P,          KB_P,
    Qt::Key_Q,          KB_Q,
    Qt::Key_R,          KB_R,
    Qt::Key_S,          KB_S,
    Qt::Key_T,          KB_T,
    Qt::Key_U,          KB_U,
    Qt::Key_V,          KB_V,
    Qt::Key_W,          KB_W,
    Qt::Key_X,          KB_X,
    Qt::Key_Y,          KB_Y,
    Qt::Key_Z,          KB_Z,

#ifdef KB_HOME
    Qt::Key_Home,       KB_HOME,
#endif // KB_HOME

#ifdef KB_HELP
    Qt::Key_F11,        KB_HELP,
#endif // KB_HELP

    0,0
};


struct mousemap
{
    int key, keynum;
    qreal left, right, top, bot;
} mouseMap[] = {

    { Qt::Key_F1,        38, 0.03, 0.15, 0.03, 0.10 },
    { Qt::Key_F2,        39, 0.20, 0.32, 0.03, 0.10 },
    { Qt::Key_F3,        40, 0.345, 0.47, 0.03, 0.10 },
    { Qt::Key_F4,        41, 0.52, 0.63, 0.03, 0.10 },
    { Qt::Key_F5,        42, 0.68, 0.80, 0.03, 0.10 },
    { Qt::Key_F6,        43, 0.83, 0.95, 0.03, 0.10 },

    { Qt::Key_A,          1, 0.03, 0.15, 0.15, 0.22 },
    { Qt::Key_B,          2, 0.20, 0.32, 0.15, 0.22 },
    { Qt::Key_C,          3, 0.345, 0.47, 0.15, 0.22 },
    { Qt::Key_D,          4, 0.52, 0.63, 0.15, 0.22 },
    { Qt::Key_E,          5, 0.68, 0.80, 0.15, 0.22 },
    { Qt::Key_F,          6, 0.83, 0.95, 0.15, 0.22 },

    { Qt::Key_G,          7, 0.03, 0.15, 0.275, 0.345 },
    { Qt::Key_H,          8, 0.20, 0.32, 0.275, 0.345 },
    { Qt::Key_I,          9, 0.345, 0.47, 0.275, 0.345 },
    { Qt::Key_J,         10, 0.52, 0.63, 0.275, 0.345 },
    { Qt::Key_K,         11, 0.68, 0.80, 0.275, 0.345 },
    { Qt::Key_L,         12, 0.83, 0.95, 0.275, 0.345 },

    { Qt::Key_Return,    13, 0.03, 0.32, 0.40, 0.47 },
    { Qt::Key_M,         14, 0.345, 0.47, 0.40, 0.47 },
    { Qt::Key_N,         15, 0.51, 0.64, 0.40, 0.47 },
    { Qt::Key_O,         16, 0.68, 0.80, 0.40, 0.47 },
    { Qt::Key_Backspace, 17, 0.83, 0.95, 0.40, 0.47 },

    { Qt::Key_Up,        18, 0.03, 0.15, 0.52, 0.59 },
    { Qt::Key_7,         19, 0.23, 0.36, 0.52, 0.59 },
    { Qt::Key_8,         20, 0.42, 0.56, 0.52, 0.59 },
    { Qt::Key_9,         21, 0.62, 0.75, 0.52, 0.59 },
    { Qt::Key_Slash,     22, 0.81, 0.95, 0.52, 0.59 },

    { Qt::Key_Down,      23, 0.03, 0.15, 0.645, 0.715 },
    { Qt::Key_4,         24, 0.23, 0.36, 0.645, 0.715 },
    { Qt::Key_5,         25, 0.42, 0.56, 0.645, 0.715 },
    { Qt::Key_6,         26, 0.62, 0.75, 0.645, 0.715 },
    { Qt::Key_Asterisk,  27, 0.81, 0.95, 0.645, 0.715 },

    { Qt::Key_Alt,       28, 0.028, 0.145, 0.77, 0.84 },
    { Qt::Key_1,         29, 0.23, 0.36, 0.77, 0.84 },
    { Qt::Key_2,         30, 0.42, 0.56, 0.77, 0.84 },
    { Qt::Key_3,         31, 0.62, 0.75, 0.77, 0.84 },
    { Qt::Key_Minus,     32, 0.81, 0.95, 0.77, 0.84 },

    { Qt::Key_Escape,    33, 0.03, 0.15, 0.89, 0.97 },
    { Qt::Key_0,         34, 0.23, 0.36, 0.89, 0.97 },
    { Qt::Key_Period,    35, 0.42, 0.55, 0.89, 0.97 },
    { Qt::Key_Question,  36, 0.62, 0.74, 0.89, 0.97 },
    { Qt::Key_Plus,      37, 0.81, 0.95, 0.89, 0.97 },

    {                0,  0,      0.0,      0.0,      0.0,      0.0}
};


void MainWindow::pushKey(int key)
// ----------------------------------------------------------------------------
//   When pushing a key, update the highlight rectangle
// ----------------------------------------------------------------------------
{
    QRect rect(0, 0, 0, 0);
    for (mousemap *ptr = mouseMap; ptr->key; ptr++)
    {
        if (ptr->keynum == key)
        {
            int w = ui.keyboard->width();
            int h = ui.keyboard->height();
            rect.setCoords(ptr->left * w, ptr->top * h,
                           ptr->right * w, ptr->bot * h);
            break;
        }
    }
    record(sim_keys,
           "Key %d coords (%d, %d, %d, %d)",
           key,
           rect.x(),
           rect.y(),
           rect.width(),
           rect.height());
    emit keyResizeSignal(rect);
}


void Highlight::keyResizeSlot(const QRect &rect)
// ----------------------------------------------------------------------------
//   Receive signal that the widget was resized
// ----------------------------------------------------------------------------
{
    setGeometry(rect);
}


void Highlight::paintEvent(QPaintEvent *)
// ----------------------------------------------------------------------------
//   Repaing, showing the highlight
// ----------------------------------------------------------------------------
{
    QRect geo = geometry();
    record(sim_keys, "Repainting %d %d %d %d",
           geo.x(), geo.y(), geo.width(), geo.height());
    QRect local(3, 3, geo.width()-6, geo.height()-6);
    QPainter p(this);
    QPainterPath path;
    path.addRoundedRect(local, 8, 8);
    QPen pen(Qt::yellow, 4);
    p.setPen(pen);
    p.drawPath(path);
}


void MainWindow::keyPressEvent(QKeyEvent * ev)
// ----------------------------------------------------------------------------
//   Got a key - Push it to the simulator
// ----------------------------------------------------------------------------
{
    if (ev->isAutoRepeat())
    {
        ev->accept();
        return;
    }

    int k = ev->key();
    record(sim_keys, "Key press %d", k);

    if (k == Qt::Key_F16)
        recorder_dump_for(tests::dump_on_fail);

    if (k == Qt::Key_F13 || k == Qt::Key_F14 || k == Qt::Key_F15 ||
        k == Qt::Key_F11 || k == Qt::Key_F12)
    {
        if (!tests.isRunning())
        {
            tests.onlyCurrent = k == Qt::Key_F11;
            tests.demo1 = k == Qt::Key_F13;
            tests.demo2 = k == Qt::Key_F14;
            tests.demo3 = k == Qt::Key_F15;
            tests.start();
        }
        else
        {
            tests.terminate();
            tests.wait();
            fprintf(stderr, "\n\n\nTests interrupted\n");
        }
    }

    if (k == Qt::Key_F10)
    {
        static cstring keyboards[] =
        {
            "config/db48x.48k",
            "config/legacy.48k",
            "config/42style.48k",
            "config/true42.48k",
        };

        // HACK - Not thread safe, don't do that while running
        extern user_interface ui;
        static uint newmap = 0;
        ui.load_keymap(keyboards[newmap++]);
        newmap %= sizeof(keyboards) / sizeof(keyboards[0]);
    }

    if (k == Qt::Key_F9)
    {
        const int header_h = 22;
        screenshot("screens/screenshot-", 0, header_h, LCD_W, LCD_H - header_h);
        ev->accept();
        return;
    }

    if (k == Qt::Key_F8)
    {
        key_push(tests::SAVE_PGM);
        return;
    }

    if (k == Qt::Key_C && (ev->modifiers() & Qt::ControlModifier))
    {
        // HACK: Not thread safe at all!
        extern user_interface ui;
        ui.clear_shift();

        QClipboard *clipboard = QApplication::clipboard();
        if (ev->modifiers() & Qt::ShiftModifier)
        {
            QPixmap &screen = MainWindow::theScreen();
            clipboard->setPixmap(screen);
        }
        else if (size_t sz = rt.editing())
        {
            utf8 data = rt.editor();
            QByteArray ba(cstring(data), sz);
            QString text(ba);
            clipboard->setText(text);
        }
        else if (!ST(STAT_RUNNING))
        {
            if (object_p obj = rt.top())
            {
                text_p sym = obj->as_text();
                size_t sz = 0;
                utf8 data = sym->value(&sz);
                QByteArray ba(cstring(data), sz);
                QString text(ba);
                clipboard->setText(text);
            }
        }
        ev->accept();
        return;
    }

    if (k == Qt::Key_V && (ev->modifiers() & Qt::ControlModifier))
    {
        // HACK: Not thread safe at all!
        extern user_interface ui;
        ui.clear_shift();

        QClipboard *clipboard = QApplication::clipboard();
        QString text = clipboard->text();
        text.replace("\r\n", "\n");
        QByteArray ba = text.toUtf8();
        if (size_t sz = ba.size())
        {
            if (!ST(STAT_RUNNING))
            {
                uint pos = ui.cursor_position();
                size_t ins = ui.insert(pos, utf8(ba.data()), sz);
                ui.cursor_position(pos+ins);
            }
        }
        ev->accept();
        return;
    }

    if (k == Qt::Key_Shift)
    {
        shift_held = true;
    }
    else if (k == Qt::Key_Alt)
    {
        alt_held = true;
    }
    else if ((k >= Qt::Key_A && k <= Qt::Key_Z)    ||
             (k >= Qt::Key_F1 && k <= Qt::Key_F6))
    {
        if (shift_held)
            key_push(KEY_UP);
        else if (alt_held)
            key_push(KEY_DOWN);
    }

    for (int i = 0; keyMap[i] != 0; i += 2)
    {
        if (k == keyMap[i])
        {
            record(sim_keys, "Key %d found at %d, DM42 key is %d",
                   k, i, keyMap[i+1]);
            key_push(keyMap[i+1]);
            ev->accept();
            return;
        }
    }

    QMainWindow::keyPressEvent(ev);
}


void MainWindow::keyReleaseEvent(QKeyEvent * ev)
// ----------------------------------------------------------------------------
//   Released a key - Send a 0 to the simulator
// ----------------------------------------------------------------------------
{
    if (ev->isAutoRepeat())
    {
        ev->accept();
        return;
    }

    int k = ev->key();
    record(sim_keys, "Key release %d", k);
    if (k == Qt::Key_Shift)
        shift_held = false;
    else if (k == Qt::Key_Alt)
        alt_held = false;

    for (int i = 0; keyMap[i] != 0; i += 2)
    {
        if (k == keyMap[i])
        {
            record(sim_keys, "Key %d found at %d, sending key up", k, i);
            key_push(0);
            ev->accept();
            return;
        }
    }

    QMainWindow::keyReleaseEvent(ev);
}


bool MainWindow::eventFilter(QObject * obj, QEvent * ev)
// ----------------------------------------------------------------------------
//  Filter mouse / keyboard events
// ----------------------------------------------------------------------------
{
    if (ev->type() != QEvent::Resize &&
        ev->type() != QEvent::Move &&
        !QApplication::mouseButtons())
    {
        record(sim_window, "Clearing resize direction %d buttons %x",
               int(ev->type()),
               QApplication::mouseButtons());
        resizeDirection = 0;
    }

    if (obj == ui.keyboard)
    {
        if ((ev->type() == QEvent::TouchBegin) ||
            (ev->type() == QEvent::TouchUpdate) ||
            (ev->type() == QEvent::TouchEnd) ||
            (ev->type() == QEvent::TouchCancel))
        {
            QTouchEvent *me = static_cast < QTouchEvent * >(ev);
#if QT_VERSION < 0x060000
            auto &touchPoints = me->touchPoints();
#else
            auto &touchPoints = me->points();
#endif // Qt version 6
            qsizetype npoints = touchPoints.count();

            record(sim_keys, "Touch event %d points", npoints);

            for(int k = 0; k < npoints; ++k) {
#if QT_VERSION < 0x060000
                QPointF coordinates = touchPoints.at(k).startPos();
#else
                QPointF coordinates = touchPoints.at(k).pressPosition();
#endif // Qt version 6
                qreal relx, rely;
                int   pressed;

                if(touchPoints.at(k).state() & Qt::TouchPointPressed)
                    pressed = 1;
                else if(touchPoints.at(k).
                        state() & Qt::TouchPointReleased)
                    pressed = 0;
                else
                    continue;   // NOT INTERESTED IN DRAGGING

                relx = coordinates.x() / (qreal) ui.keyboard->width();
                rely = coordinates.y() / (qreal) ui.keyboard->height();
                record(sim_keys, "  [%d] at (%f, %f) %+s",
                       k, relx, rely, pressed ? "pressed" : "released");

                if (!pressed)
                    key_push(0);
                else
                    for (mousemap *ptr = mouseMap; ptr->key; ptr++)
                        if ((relx >= ptr->left) && (relx <= ptr->right) &&
                            (rely >= ptr->top) && (rely <= ptr->bot))
                        {
                            record(sim_keys, "  [%d] found at %d as %d",
                                   k, ptr - mouseMap, ptr->keynum);
                            key_push(ptr->keynum);
                        }
            }

            return true;
        }

        if (ev->type() == QEvent::MouseButtonPress ||
            ev->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent *me = static_cast < QMouseEvent * >(ev);
#if QT_VERSION < 0x060000
            qreal relx = (qreal) me->x() / (qreal) ui.keyboard->width();
            qreal rely = (qreal) me->y() / (qreal) ui.keyboard->height();
#else
            qreal relx =
                (qreal) me->position().x() / (qreal) ui.keyboard->width();
            qreal rely =
                (qreal) me->position().y() / (qreal) ui.keyboard->height();
#endif // Qt vertsion 6

            record(sim_keys, "Mouse button press at (%f, %f)", relx, rely);
            for (mousemap *ptr = mouseMap; ptr->key; ptr++)
                if ((relx >= ptr->left) && (relx <= ptr->right) &&
                    (rely >= ptr->top) && (rely <= ptr->bot))
                {
                    record(sim_keys, "Mouse coordinates found at %d as %d",
                           ptr - mouseMap, ptr->keynum);

                    key_push(ptr->keynum);
                }

            return true;
        }

        if(ev->type() == QEvent::MouseButtonRelease)
        {
            record(sim_keys, "Mouse button released");
            key_push(0);
            return true;
        }

        return false;
    }

    return false;
}


void MainWindow::screenshot(cstring basename, int x, int y, int w, int h)
// ----------------------------------------------------------------------------
//   Save a simulator screenshot under the "SCREEN" directory
// ----------------------------------------------------------------------------
{
    QString   name  = basename;
    QDateTime today = QDateTime::currentDateTime();
    name += today.toString("yyyyMMdd-hhmmss");
    name += ".png";

    QPixmap &screen = MainWindow::theScreen();
    QPixmap img = screen.copy(x, y, w, h);
    bool ok = img.save(name, "PNG");
    record(sim_window, "Screen capture %+s for %s",
           ok ? "succeeded" : "failed",
           name.toUtf8().constData());
}


void MainWindow::load_keymap(cstring keymapfile)
// ----------------------------------------------------------------------------
//   A new keymap was loaded, update visible keyboard layout on screen
// ----------------------------------------------------------------------------
{
    QFileInfo fi(keymapfile);
    QString name = fi.baseName() + ".png";
    QString style = ("border-image: url(:/bitmap/" + name + ") "
                     "0 0 0 0 stretch stretch;");
    theMainWindow()->ui.keyboard->setStyleSheet(style);
}



// ============================================================================
//
//   AudioGenerator: Generate audio output on demand
//
// ============================================================================

AudioGenerator::AudioGenerator(const QAudioFormat &format,
                               qint64              durationUs,
                               uint                freq)
// ----------------------------------------------------------------------------
//   Constructor for the audio generator
// ----------------------------------------------------------------------------
    : freq(freq)
{
    if (format.isValid())
        generateData(format, durationUs, freq);
}


void AudioGenerator::start()
// ----------------------------------------------------------------------------
//   Start generating samples
// ----------------------------------------------------------------------------
{
    open(QIODevice::ReadOnly);
}


void AudioGenerator::stop()
// ----------------------------------------------------------------------------
//   Stop generating samples
// ----------------------------------------------------------------------------
{
    pos = 0;
    close();
}


template <typename Data>
static inline void generate(char  *buffer,
                            size_t frames,
                            uint   channels,
                            uint   freq,
                            uint   sampleRate,
                            double scale,
                            double offset)
// ----------------------------------------------------------------------------
//  Generate data to a sample buffer
// ----------------------------------------------------------------------------
{
    Data  *ptr  = (Data *) buffer;
    double amp = 0.5;
    double rate = (2 * M_PI / 1000) * freq / sampleRate;
    scale *= amp;
    for (size_t f = 0; f < frames; f++)
    {
        double x = sin(rate * (f % sampleRate));
        // x *= sin(0.125 * rate * (f % sampleRate));
        x = x > 0 ? 1 : -1;
        x = x * scale + offset;
        Data sample = Data(x);
        for (uint c = 0; c < channels; c++)
            *ptr++ = sample;
    }
}


void AudioGenerator::generateData(const QAudioFormat &format,
                                  qint64              durationUs,
                                  uint                 freq)
// ----------------------------------------------------------------------------
//    Generating data for the samples
// ----------------------------------------------------------------------------
{
    uint   frameBytes   = format.bytesPerFrame();
    uint   channels     = format.channelCount();
    qint64 frames       = format.framesForDuration(durationUs);
    size_t bytes        = frames * frameBytes;
    int    sampleRate   = format.sampleRate();

    buffer.resize(bytes);
    char *start = buffer.data();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto   sampleFormat = format.sampleFormat();
    switch (sampleFormat)
    {
    default:
    case QAudioFormat::UInt8:
        generate<quint8>(start, frames, channels, freq, sampleRate, 255./2, 255./2);
        break;
    case QAudioFormat::Int16:
        generate<qint16>(start, frames, channels, freq, sampleRate, 32767, 0);
        break;
    case QAudioFormat::Int32:
        generate<qint32>(start, frames, channels, freq, sampleRate,
                     std::numeric_limits<qint32>::max(), 0);
        break;
    case QAudioFormat::Float:
        generate<float>(start, frames, channels, freq, sampleRate, 1.0, 0.0);
        break;
    }
#else
    // Qt5 uses sampleSize() and sampleType() instead of sampleFormat()
    int sampleSize = format.sampleSize();
    QAudioFormat::SampleType sampleType = format.sampleType();

    if (sampleType == QAudioFormat::UnSignedInt && sampleSize == 8)
    {
        generate<quint8>(start, frames, channels, freq, sampleRate, 255./2, 255./2);
    }
    else if (sampleType == QAudioFormat::SignedInt && sampleSize == 16)
    {
        generate<qint16>(start, frames, channels, freq, sampleRate, 32767, 0);
    }
    else if (sampleType == QAudioFormat::SignedInt && sampleSize == 32)
    {
        generate<qint32>(start, frames, channels, freq, sampleRate,
                     std::numeric_limits<qint32>::max(), 0);
    }
    else if (sampleType == QAudioFormat::Float)
    {
        generate<float>(start, frames, channels, freq, sampleRate, 1.0, 0.0);
    }
    else
    {
        // Default fallback
        generate<qint16>(start, frames, channels, freq, sampleRate, 32767, 0);
    }
#endif
}


qint64 AudioGenerator::readData(char *data, qint64 len)
// ----------------------------------------------------------------------------
//   Read data from the buffer
// ----------------------------------------------------------------------------
{
    qint64 total = 0;
    if (!buffer.isEmpty())
    {
        while (len - total > 0)
        {
            qint64 chunk = qMin((buffer.size() - pos), len - total);
            memcpy(data + total, buffer.constData() + pos, chunk);
            pos = (pos + chunk) % buffer.size();
            total += chunk;
        }
    }
    return total;
}


qint64 AudioGenerator::writeData(const char *data, qint64 len)
// ----------------------------------------------------------------------------
//   We don't write data in our use case
// ----------------------------------------------------------------------------
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}


qint64 AudioGenerator::bytesAvailable() const
// ----------------------------------------------------------------------------
//   Return number of bytes available in the generator
// ----------------------------------------------------------------------------
{
    return buffer.size() + QIODevice::bytesAvailable();
}


#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void MainWindow::initializeAudio(const QAudioDevice &deviceInfo, uint freq)
#else
void MainWindow::initializeAudio(const QAudioDeviceInfo &deviceInfo, uint freq)
#endif
// ----------------------------------------------------------------------------
//   Audio setup for the simulator
// ----------------------------------------------------------------------------
{
    QAudioFormat format = deviceInfo.preferredFormat();
    const int    durationUs = 1000000 /* microseconds */;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    audio.reset(new QAudioSink(deviceInfo, format));
#else
    audio.reset(new QAudioOutput(deviceInfo, format));
#endif
    generator.reset(new AudioGenerator(format, durationUs, freq));
    generator->start();
    audio->setVolume(0);
    audio->start(generator.data());
}


void MainWindow::startBuzzer(uint frequency)
// ----------------------------------------------------------------------------
//   Start a buzzer
// ----------------------------------------------------------------------------
{
    record(sim_audio, "Start buzzer %d.%02d Hz, creating samples",
           frequency / 100, frequency % 100);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    initializeAudio(devices->defaultAudioOutput(), frequency);
#else
    initializeAudio(QAudioDeviceInfo::defaultOutputDevice(), frequency);
#endif
    audio->setVolume(1);
    switch (audio->state())
    {

    case QAudio::SuspendedState:
    case QAudio::StoppedState:
    case QAudio::IdleState:
        audio->resume();
        break;
    default:
    case QAudio::ActiveState:
        // no-op
        break;
    }
    playing = true;
}


void MainWindow::stopBuzzer()
// ----------------------------------------------------------------------------
//   Start a buzzer
// ----------------------------------------------------------------------------
{
    record(sim_audio, "Stop buzzer, audio state is %d", audio->state());
    switch (audio->state())
    {
    default:
    case QAudio::SuspendedState:
    case QAudio::StoppedState:
    case QAudio::IdleState:
        // No-op
        break;
    case QAudio::ActiveState:
        audio->suspend();
        break;
    }
    playing = false;
}


void MainWindow::updateAudioDevices()
// ----------------------------------------------------------------------------
//   Audio devices changed, restart without changing the frequency
// ----------------------------------------------------------------------------
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    initializeAudio(devices->defaultAudioOutput(), generator->frequency());
#else
    initializeAudio(QAudioDeviceInfo::defaultOutputDevice(), generator->frequency());
#endif
}



// ============================================================================
//
//   Interface with DMCP and the test harness
//
// ============================================================================

void ui_refresh()
// ----------------------------------------------------------------------------
//   Request a refresh of the LCD
// ----------------------------------------------------------------------------
{
    static uint done = true;
    while (!done) sys_delay(1);
    done = false;
    SimScreen::update_pixmap();
    postToThread([&] { SimScreen::refresh_lcd(); done = true; });
}


uint ui_refresh_count()
// ----------------------------------------------------------------------------
//   Return the number of times the display was actually udpated
// ----------------------------------------------------------------------------
{
    return SimScreen::redraw_count();
}


void ui_screenshot()
// ----------------------------------------------------------------------------
//   Take a screen snapshot
// ----------------------------------------------------------------------------
{
    MainWindow::screenshot();
}


void ui_push_key(int k)
// ----------------------------------------------------------------------------
//   Update display when pushing a key
// ----------------------------------------------------------------------------
{
    MainWindow::theMainWindow()->pushKey(k);
}


void ui_ms_sleep(uint ms_delay)
// ----------------------------------------------------------------------------
//   Suspend the current thread for the given interval in milliseconds
// ----------------------------------------------------------------------------
{
    QThread::msleep(ms_delay);
}


int ui_file_selector(const char *title,
                     const char *base_dir,
                     const char *ext,
                     file_sel_fn callback,
                     void       *data,
                     int         disp_new,
                     int         overwrite_check)
// ----------------------------------------------------------------------------
//  File selector function
// ----------------------------------------------------------------------------
{
#ifdef ANDROID
    // Engage the lock. If it was already true (e.g. touch bounce), safely abort.
    if (is_dialog_open.exchange(true)) {
        return MRET_EXIT;
    }
#endif

    QString path;
    bool done = false;

#ifdef ANDROID
    // Android SAF rejects absolute Linux paths like "/state".
    // We must use an empty string so the OS opens its default safe location.
    QString initial_dir = "";
    // Android requires a valid parent context to launch the intent.
    QWidget* parent_widget = MainWindow::theMainWindow();
#else
    QString initial_dir = base_dir;
    QWidget* parent_widget = nullptr;
#endif

    postToThread([&]{ // the functor captures parent and text by value
        path =
            disp_new
            ? QFileDialog::getSaveFileName(parent_widget,
                                           title,
                                           initial_dir,
                                           QString("*") + QString(ext),
                                           nullptr,
                                           overwrite_check
                                           ? QFileDialog::Options()
                                           : QFileDialog::DontConfirmOverwrite)
            : QFileDialog::getOpenFileName(parent_widget,
                                           title,
                                           initial_dir,
                                           QString("*") + QString(ext));
        std::cout << "Selected path: " << path.toStdString() << "\n";
        done = true;
    });

    while (!done)
        sys_delay(50);

    int ret = MRET_EXIT;
    if (!path.isNull())
    {
        QFileInfo fi(path);
        QString suffix = fi.suffix(); // On Linux we don't get the extension
        QString name = fi.fileName();
        path = fi.absoluteFilePath();
#ifdef ANDROID
        // Create a persistent, private sandbox path that standard C++ can read/write
        // This requires no permissions and survives app restarts.
        QString sandboxDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(sandboxDir); // Ensure the directory exists
        QString sandboxPath = sandboxDir + "/" + name;

        if (!disp_new)
        {
            // LOADING (Import): The user selected a file via the Android picker.
            // Use Qt to copy the Android URI data into our POSIX sandbox file.
            QFile::remove(sandboxPath);
            QFile::copy(path, sandboxPath);

            // Tell the DB48X engine to load from the sandbox.
            ret = callback(sandboxPath.toStdString().c_str(), name.toStdString().c_str(), data);
        }
        else
        {
            // SAVING (Export): Tell DB48X to save its state to the POSIX sandbox file.
            ret = callback(sandboxPath.toStdString().c_str(), name.toStdString().c_str(), data);

            // If the DB48X engine succeeded, use Qt to copy the sandbox file
            // out to the public Android URI the user selected.
            if (ret == MRET_EXIT) {
                QFile targetFile(path);
                if (targetFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    QFile internalFile(sandboxPath);
                    if (internalFile.open(QIODevice::ReadOnly)) {
                        targetFile.write(internalFile.readAll());
                        internalFile.close();
                    }
                    targetFile.close();
                }
            }
        }
#else
        // --- Desktop Behavior ---
        if (QFileInfo("." + suffix) != QFileInfo(ext))
        {
            path += ext;
            name += ext;
        }
        QString here = QDir::currentPath() + QDir::separator();
        if (path.startsWith(here))
            path = path.remove(0, here.length());
        std::cout << "Got path: " << path.toStdString()
                  << ", name is " << name.toStdString() << "\n";
        ret = callback(path.toStdString().c_str(),
                       name.toStdString().c_str(),
                       data);
#endif
    }

#ifdef ANDROID
    is_dialog_open = false; // Release the lock
#endif

    return ret;
}


void ui_save_setting(const char *name, const char *value)
// ----------------------------------------------------------------------------
//  Save some settings
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.setValue(name, value);
}


size_t ui_read_setting(const char *name, char *value, size_t maxlen)
// ----------------------------------------------------------------------------
//  Save some settings
// ----------------------------------------------------------------------------
{
    QSettings settings;
    QString current = settings.value(name).toString();
    if (current.isNull())
        return 0;
    if (value)
        strncpy(value, current.toStdString().c_str(), maxlen);
    return current.length();
}


uint last_battery_ms = 0;
uint battery = 1000;
bool charging = false;

uint ui_battery()
// ----------------------------------------------------------------------------
//   Return the battery voltage
// ----------------------------------------------------------------------------
{
    const uint vmax = 3000;
    const uint vmin = 2600;
    const uint vlow = (vmax + 3 * vmin) / 4;

    uint now = sys_current_ms();
    if (last_battery_ms < now - 1000)
        last_battery_ms = now - 1000;

    if (charging)
    {
        battery += (1000 - battery) * (now - last_battery_ms) / 6000;
        if (battery >= 990)
            charging = false;
    }
    else
    {
        battery -= (now - last_battery_ms) / 10;
        uint v = battery * (vmax - vmin) / 1000 + vmin;
        if (v < vlow)
            charging = true;
    }

    last_battery_ms = now;
    return battery;
}


bool ui_charging()
// ----------------------------------------------------------------------------
//   Return true if USB-powered or not
// ----------------------------------------------------------------------------
{
    return charging;
}


void ui_start_buzzer(uint frequency)
// ----------------------------------------------------------------------------
//   Start buzzer at given frequency
// ----------------------------------------------------------------------------
{
    MainWindow *main = MainWindow::theMainWindow();
    if (main->buzzerPlaying())
        ui_stop_buzzer();

    postToThread([&] { main->startBuzzer(frequency); });

    while (!main->buzzerPlaying())
        sys_delay(20);
}


void ui_stop_buzzer()
// ----------------------------------------------------------------------------
//  Stop buzzer in simulator
// ----------------------------------------------------------------------------
{
    MainWindow *main = MainWindow::theMainWindow();
    postToThread([&] { main->stopBuzzer(); });
    while (main->buzzerPlaying())
        sys_delay(20);
}


int ui_wrap_io(file_sel_fn callback, const char *path, void *data, bool)
// ----------------------------------------------------------------------------
//   Wrap I/Os into thread safety / file sync
// ----------------------------------------------------------------------------
{
    cstring name = path;
    for (cstring p = path; *p; p++)
        if (*p == '/' || *p == '\\')
            name = p + 1;
    return callback(path, name, data);
}

void ui_load_keymap(cstring name)
// ----------------------------------------------------------------------------
//   Change the visible keyboard layout
// ----------------------------------------------------------------------------
{
    MainWindow::load_keymap(name);
}


RECORDER(image_check,       16, "Comparison of images in Qt");
RECORDER(image_check_error, 16, "Error comparing images in Qt");
extern QDir testDirectory;

bool tests::image_match(cstring file, int x, int y, int w, int h, bool force)
// ----------------------------------------------------------------------------
//   Check if the screen matches a given file
// ----------------------------------------------------------------------------
{
    QPixmap &screen = MainWindow::theScreen();
    QPixmap img = screen.copy(x, y, w, h);
    QPixmap data;
    QString name = force ? "images/bad/" : "images/";
    name += file;
    name += ".png";
#ifdef CONFIG_COLOR
    name = "color-" + name;
#  endif // CONFIG_COLOR
    name = testDirectory.filePath(name);
    QFileInfo reference(name);
    if (force || !reference.exists() || !data.load(name, "PNG"))
    {
        bool result = img.save(name, "PNG");
        if (!result)
            record(image_check_error, "Can't save image: %s", strerror(errno));
        return result;
    }
    bool ok         = data.toImage() == img.toImage();
    uint mismatched = 0;
    if (!ok)
    {
        // Workaround for what appears to be a Qt bug, seen on Input test
        const auto &r = data.toImage();
        const auto &i = img.toImage();
        auto rw = r.width();
        auto rh = r.height();
        auto iw = i.width();
        auto ih = i.height();
        ok = true;
        if (rw != iw || rh != ih)
        {
            ok = false;
        }
        else
        {
            for (int x = 0; x < rw; x++)
            {
                for (int y = 0; y < rh; y++)
                {
                    QRgb rc = r.pixel(x, y);
                    QRgb ic = i.pixel(x, y);
                    if (rc != ic)
                        mismatched++;
                }
            }
            ok = mismatched < 5;
        }
        record(image_check, "%+s: %u mismatched", file, mismatched);
    }
    return ok;
}

#else // WASM

// ============================================================================
//
//   Platform support on WASM
//
// ============================================================================

static uint refresh_count = 0;

RECORDER(wasm, 16, "WASM interface");

void ui_refresh()
// ----------------------------------------------------------------------------
//   Request a refresh of the LCD
// ----------------------------------------------------------------------------
{
    refresh_count++;
    record(wasm, "Refresh count=%u", refresh_count);
    wasm_updated_screen = uintptr_t(lcd_buffer);
}


uint ui_refresh_count()
// ----------------------------------------------------------------------------
//   Return the number of times the display was actually udpated
// ----------------------------------------------------------------------------
{
    return refresh_count;
}


void ui_screenshot()
// ----------------------------------------------------------------------------
//   Take a screen snapshot
// ----------------------------------------------------------------------------
{

}


void ui_push_key(int k)
// ----------------------------------------------------------------------------
//   Update display when pushing a key
// ----------------------------------------------------------------------------
{
//    key_push(k);
}


void ui_ms_sleep(uint ms_delay)
// ----------------------------------------------------------------------------
//   Suspend the current thread for the given interval in milliseconds
// ----------------------------------------------------------------------------
{
#ifdef WASM
    emscripten_sleep(ms_delay);
#endif // WASM
}


int ui_file_selector(const char *title,
                     const char *base_dir,
                     const char *ext,
                     file_sel_fn callback,
                     void       *data,
                     int         disp_new,
                     int         overwrite_check)
// ----------------------------------------------------------------------------
//  File selector function
// ----------------------------------------------------------------------------
{
    return 0;
}


void ui_save_setting(const char *name, const char *value)
// ----------------------------------------------------------------------------
//  Save some settings
// ----------------------------------------------------------------------------
{
}


size_t ui_read_setting(const char *name, char *value, size_t maxlen)
// ----------------------------------------------------------------------------
//  Save some settings
// ----------------------------------------------------------------------------
{
    return 0;
}


uint last_battery_ms = 0;
uint battery = 1000;
bool charging = false;

uint ui_battery()
// ----------------------------------------------------------------------------
//   Return the battery voltage
// ----------------------------------------------------------------------------
{
    const uint vmax = 3000;
    const uint vmin = 2600;
    const uint vlow = (vmax + 3 * vmin) / 4;

    uint now = sys_current_ms();
    if (last_battery_ms < now - 1000)
        last_battery_ms = now - 1000;

    if (charging)
    {
        battery += (1000 - battery) * (now - last_battery_ms) / 6000;
        if (battery >= 990)
            charging = false;
    }
    else
    {
        battery -= (now - last_battery_ms) / 10;
        uint v = battery * (vmax - vmin) / 1000 + vmin;
        if (v < vlow)
            charging = true;
    }

    last_battery_ms = now;
    return battery;
}


bool ui_charging()
// ----------------------------------------------------------------------------
//   Return true if USB-powered or not
// ----------------------------------------------------------------------------
{
    return charging;
}


void ui_start_buzzer(uint frequency)
// ----------------------------------------------------------------------------
//   Start buzzer at given frequency
// ----------------------------------------------------------------------------
{
}


void ui_stop_buzzer()
// ----------------------------------------------------------------------------
//  Stop buzzer in simulator
// ----------------------------------------------------------------------------
{
}


int ui_wrap_io(file_sel_fn callback, const char *path, void *data, bool)
// ----------------------------------------------------------------------------
//   Wrap I/Os into thread safety / file sync
// ----------------------------------------------------------------------------
{
    cstring name = path;
    for (cstring p = path; *p; p++)
        if (*p == '/' || *p == '\\')
            name = p + 1;
    return callback(path, name, data);
}

void ui_load_keymap(cstring name)
// ----------------------------------------------------------------------------
//   Change the visible keyboard layout
// ----------------------------------------------------------------------------
{
}

#endif // WASM
