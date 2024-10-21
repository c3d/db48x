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
#include "target.h"
#include "tests.h"
#include "user_interface.h"

#if WASM
#include "emcc.h"
#else
#include "sim-rpl.h"
#include "ui_sim-window.h"
#include <iostream>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioSink>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMediaDevices>
#include <QMessageBox>
#include <QStandardPaths>
#include <QtCore>
#include <QtGui>
#include <QtMath>
#endif // WASM


RECORDER(sim_keys, 16, "Recorder keys from the simulator");
RECORDER(sim_audio, 16, "Recorder keys from the simulator");

extern bool run_tests;
extern bool db48x_keyboard;
extern bool shift_held;
extern bool alt_held;

#if !WASM

MainWindow *MainWindow::mainWindow = nullptr;
qreal MainWindow::devicePixelRatio = 1.0;

MainWindow::MainWindow(QWidget *parent)
// ----------------------------------------------------------------------------
//    The main window of the simulator
// ----------------------------------------------------------------------------
    : QMainWindow(parent), ui(), rpl(this), tests(this), highlight(),
      devices(new QMediaDevices(this)), generator(), audio()
{
    mainWindow = this;

    QCoreApplication::setOrganizationName("DB48X");
    QCoreApplication::setApplicationName(PROGRAM_NAME);

    ui.setupUi(this);

    ui.keyboard->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.keyboard->installEventFilter(this);
    ui.screen->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.screen->installEventFilter(this);
    if (db48x_keyboard)
        ui.keyboard->setStyleSheet("border-image: "
                                   "url(:/bitmap/keyboard-db48x.png) "
                                   "0 0 0 0 stretch stretch;");
    else
        ui.keyboard->setStyleSheet("border-image: "
                                   "url(:/bitmap/keyboard.png) "
                                   "0 0 0 0 stretch stretch;");

    highlight = new Highlight(ui.keyboard);
    highlight->setGeometry(0,0,0,0);
    highlight->show();

    setWindowTitle(PROGRAM_NAME);

    QObject::connect(this, SIGNAL(keyResizeSignal(const QRect &)),
                     highlight, SLOT(keyResizeSlot(const QRect &)));

    qreal dpratio = qApp->primaryScreen()->devicePixelRatio();
    dpratio *= devicePixelRatio;
    resize(210 * dpratio, 370 * dpratio);

    // Audio setup
    connect(devices, &QMediaDevices::audioOutputsChanged,
            this, &MainWindow::updateAudioDevices);
    initializeAudio(devices->defaultAudioOutput(), 0);

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
    qreal dpratio = qApp->primaryScreen()->devicePixelRatio();
    int w = ui.screen->screen_width;
    int h = ui.screen->screen_height + 5;
    if(!h)
        h = LCD_H+5;
    if(!w)
        w = LCD_W;
    qreal dpwidth = event->size().width();
    qreal realwidth = dpwidth * dpratio;
    qreal scale = realwidth / w;
    if((int)scale < 1)
        scale = 1.0;
    else
        scale = (int)scale;
    if (event->size().height() * 0.38 * dpratio < scale * h)
    {
        scale = event->size().height() * 0.38 * dpratio / h;
        if ((int) scale < 1)
            scale = 1.0;
        else
            scale = (int) scale;
    }
    ui.screen->setScale(scale / dpratio);
}


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

    if (k == Qt::Key_F7 || k == Qt::Key_F8 || k == Qt::Key_F9 ||
        k == Qt::Key_F11 || k == Qt::Key_F12)
    {
        if (!tests.isRunning())
        {
            tests.onlyCurrent = k == Qt::Key_F11;
            tests.demo1 = k == Qt::Key_F7;
            tests.demo2 = k == Qt::Key_F8;
            tests.demo3 = k == Qt::Key_F9;
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
        db48x_keyboard = !db48x_keyboard;
        cstring name = db48x_keyboard
            ? ("border-image: url(:/bitmap/keyboard-db48x.png) "
               "0 0 0 0 stretch stretch;")
            : ("border-image: url(:/bitmap/keyboard.png) "
               "0 0 0 0 stretch stretch;");
        ui.keyboard->setStyleSheet(name);
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
    if(ev->isAutoRepeat()) {
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
    if(obj == ui.keyboard) {
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
    QPixmap &screen = MainWindow::theScreen();
    QPixmap img = screen.copy(x, y, w, h);
    QDateTime today = QDateTime::currentDateTime();
    QString name = basename;
    name += today.toString("yyyyMMdd-hhmmss");
    name += ".png";
    img.save(name, "PNG");
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
    auto   sampleFormat = format.sampleFormat();

    buffer.resize(bytes);
    char *start = buffer.data();

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


void MainWindow::initializeAudio(const QAudioDevice &deviceInfo, uint freq)
// ----------------------------------------------------------------------------
//   Audio setup for the simulator
// ----------------------------------------------------------------------------
{
    QAudioFormat format = deviceInfo.preferredFormat();
    const int    durationUs = 1000000 /* microseconds */;

    if (audio)
        audio->stop();
    audio.reset(new QAudioSink(deviceInfo, format));
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

    initializeAudio(devices->defaultAudioOutput(), frequency);
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
    initializeAudio(devices->defaultAudioOutput(), generator->frequency());
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
    SimScreen::update_pixmap();
    postToThread([&] { SimScreen::refresh_lcd(); });
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
    QString path;
    bool done = false;

    postToThread([&]{ // the functor captures parent and text by value
        path =
            disp_new
            ? QFileDialog::getSaveFileName(nullptr,
                                           title,
                                           base_dir,
                                           QString("*") + QString(ext),
                                           nullptr,
                                           overwrite_check
                                           ? QFileDialog::Options()
                                           : QFileDialog::DontConfirmOverwrite)
            : QFileDialog::getOpenFileName(nullptr,
                                           title,
                                           base_dir,
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
        if ("." + suffix != ext)
        {
            path += ext;
            name += ext;
        }
        std::cout << "Got path: " << path.toStdString()
                  << ", name is " << name.toStdString() << "\n";
        ret = callback(path.toStdString().c_str(),
                       name.toStdString().c_str(),
                       data);
    }
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
        uint v = battery * (BATTERY_VMAX - BATTERY_VMIN) / 1000 + BATTERY_VMIN;
        if (v < BATTERY_VLOW)
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
#endif // CONFIG_COLOR
    QFileInfo reference(name);
    if (force || !reference.exists() || !data.load(name, "PNG"))
    {
        img.save(name, "PNG");
        return true;
    }
    return data.toImage() == img.toImage();
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
        uint v = battery * (BATTERY_VMAX - BATTERY_VMIN) / 1000 + BATTERY_VMIN;
        if (v < BATTERY_VLOW)
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


#endif // WASM
