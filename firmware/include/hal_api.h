/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef _HAL_API_H
#define _HAL_API_H

#include "newrpl.h"
#include "xgl.h"
#include "usb.h"

#ifndef EXTERN
#define EXTERN extern
#endif

#ifndef NULL
#define NULL 0
#endif

//! Type to use with all timer routines: ticks and frequencies are 64-bit
typedef long long tmr_t;
//! Handle to refer to a timed event.
typedef int HEVENT;

typedef struct
{
    unsigned int Prolog;
    unsigned short BitmapWidth;
    unsigned short BitmapHeight;
    unsigned short OffsetBitmap;
    unsigned short OffsetTable;
    unsigned int MapTable[];
} UNIFONT;

// COMPACT TIME STRUCTURE, SIMILAR TO STANDARD tm BUT USING ONLY 64 BITS.
struct compact_tm
{
    unsigned tm_sec:6,  //seconds after the minute      0-60*
        tm_min:6,       //minutes after the hour        0-59
        tm_hour:5,      //hours since midnight  0-23
        tm_mday:5,      //day of the month      1-31
        tm_mon:4,       //months since January  0-11
        tm_wday:3,      //days since Sunday     0-6
        tm_isdst:1;
    int tm_year;        //int     years since 1900
};

typedef gglsurface DRAWSURFACE;

enum keyContext
{
    IN_STACK = 0,
    IN_CMDLINE,
    IN_FORM,
    IN_USER = 16        // use IN_USER+NNN FOR CUSTOM KEY CONTEXT FOR APPLICATIONS
};

enum halFlagsEnum
{
    HAL_FASTMODE = 1,
    HAL_SLOWLOCK = 2,
    HAL_HOURGLASS = 4,
    HAL_AUTOOFFTIME1 = 8,
    HAL_AUTOOFFTIME2 = 16,
    HAL_AUTOOFFTIME3 = 32,
    HAL_NOCLOCKCHANGE = 64,
    HAL_SKIPNEXTALARM = 128,
    HAL_HALTED = 256,
    HAL_TIMEOUT = 512,
    HAL_AUTORESUME = 1024,
    HAL_FASTAUTORESUME = 2048,
    HAL_POWEROFF = 4096,
    HAL_RESET = 8192,
    HAL_HWRESET = 16384,
    HAL_QUICKRESPONSE = 32768
            // ADD MORE BITS HERE
            // STARTING WITH 2^16 = 65536 ARE NOTIFICATION FLAGS
};

#define HAL_AUTOOFFTIME (HAL_AUTOOFFTIME1|HAL_AUTOOFFTIME2|HAL_AUTOOFFTIME3)
#define GET_AUTOOFFTIME(flags) (((flags)&HAL_AUTOOFFTIME)/HAL_AUTOOFFTIME1)
#define SET_AUTOOFFTIME(ntimes15) ((ntimes15*HAL_AUTOOFFTIME1)&HAL_AUTOOFFTIME)

// ANNUNCIATORS

/*
 * (131, 0) - Remote
 * (131, 1) - Left Shift
 * (131, 2) - Right Shift
 * (131, 3) - Alpha
 * (131, 4) - Low Battery
 * (131, 5) - Wait
*/
enum halNotification
{
    N_CONNECTION = 0,
    N_LEFTSHIFT,
    N_RIGHTSHIFT,
    N_ALPHA,
    N_LOWBATTERY,
    N_HOURGLASS,
    N_DATARECVD,
    N_ALARM,
    N_INTERNALSHIFTHOLD,
    N_INTERNALALPHAHOLD
};

enum halFonts
{
    FONT_STACK = 0,
    FONT_STACKLVL1,
    FONT_CMDLINE,
    FONT_MENU,
    FONT_STATUS,
    FONT_PLOT,
    FONT_FORMS,
    FONT_HLPTEXT,
    FONT_HLPTITLE,
    FONTS_NUM
};

#define FORM_DIRTY 1
#define STACK_DIRTY 2
#define CMDLINE_DIRTY 4
#define CMDLINE_LINEDIRTY  8
#define CMDLINE_CURSORDIRTY 16
#define CMDLINE_ALLDIRTY   (4+8+16)
#define MENU1_DIRTY 32
#define MENU2_DIRTY 64
#define STAREA_DIRTY 128
#define BUFFER_LOCK  16384
#define BUFFER_ALT   32768


// NUMBER OF ENTRIES IN THE RENDER CACHE

#define MAX_RENDERCACHE_ENTRIES 32

// STRUCT TO CONTAIN THE HEIGHT IN PIXELS OF SCREEN AREAS (0=INVISIBLE)
typedef struct
{
    int Form;
    int Stack;
    int CmdLine;
    int Menu1;
    int Menu2;
    int HelpMode;       // SOFT MENU ON-SCREEN HELP
    int DirtyFlag;      // 1 BIT PER AREA IN ORDER, 1=FORM, 2=STACK, 4=CMDLINE, 8=MENU1,16=MENU2,32=STATUS
    HEVENT SAreaTimer, CursorTimer;
    UNIFONT const **FontArray[FONTS_NUM];     // POINTERS TO ADDRESSES OF FONTS
    WORD FontHash[FONTS_NUM];   // HASH TO DETECT FONT CHANGES
    // VARIABLES FOR THE TEXT EDITOR / COMMAND LINE
    int LineVisible, LineCurrent, LineIsModified;
    int NumLinesVisible;        // HEIGHT OF COMMAND LINE AREA IN LINES OF TEXT
    int CursorState;    // Lowercase, Uppercase, Token, VISIBLE OR INVISIBLE
    int CursorPosition; // OFFSET FROM START OF CURRENT LINE
    int CursorX, XVisible;
    int SelStart, SelEnd;       // CURRENT SELECTION START/END OFFSET WITHIN THE LINE
    int SelStartLine, SelEndLine;       // CURRENT SELECTION START/END LINE (-1 IF NO SELECTION)
    int CmdLineState;   // STATUS FLAGS FOR THE COMMAND LINE
    int CmdLineIndent;  // INDENT SPACING
    int ACTokenStart;   // START OF TOKEN FOR AUTO COMPLETE, OFFSET FROM START OF LINE
    WORD ACSuggestion;  // CURRENT SUGGESTED OPCODE

    // VARIABLES FOR USER INTERFACE
    int StkUndolevels, StkCurrentLevel;
    int KeyContext;

    // INTERACTIVE STACK VARIABLES
    int StkPointer, StkVisibleLvl, StkSelStart, StkSelEnd;
    int StkVisibleOffset, StkSelStatus;

    // DEFERRED IDLE PROCESSES
    void (*Processes[3])(void);

} HALSCREEN;

extern HALSCREEN halScreen;

// CALCULATOR CONTEXT IDENTIFIERS
enum
{
    // FIRST NUMBERS, UP TO 32 ARE BIT-FIELDS AND CAN BE ACTIVATED WITHIN
    // OTHER CONTEXTS
    CONTEXT_ANY = 0,
    CONTEXT_INEDITOR = 1,
    CONTEXT_STACK = 2,
    CONTEXT_INTSTACK = 4,
    CONTEXT_PLOT = 8,
    CONTEXT_PICT = 16,

    // CONTEXT NUMBERS ABOVE 32 ARE JUST NUMBERS (MUST BE MULTIPLE OF 32)
    CONTEXT_FORM = 32
            // ADD MORE SYSTEM CONTEXTS HERE
};

#define CMDSTATE_OPEN        0x100
#define CMDSTATE_FULLSCREEN  0x200
#define CMDSTATE_SELBEGIN    0x400
#define CMDSTATE_SELEND      0x800
#define CMDSTATE_SELECTION   0xC00
#define CMDSTATE_OVERWRITE  0x1000
#define CMDSTATE_ACACTIVE   0x2000
#define CMDSTATE_ACUPDATE   0x4000

// Type definition for interrupt handler functions

typedef void (*__interrupt__)(void);

enum
{
    MEM_AREA_RSTK = 0,
    MEM_AREA_DSTK,
    MEM_AREA_DIR,
    MEM_AREA_LAM,
    MEM_AREA_TEMPOB,
    MEM_AREA_TEMPBLOCKS
};



// MAIN EXCEPTION PROCESSOR

#define __EX_CONT 1     // SHOW CONTINUE OPTION
#define __EX_EXIT 2     // SHOW EXIT OPTION
#define __EX_WARM 4     // SHOW WARMSTART OPTION
#define __EX_RESET 8    // SHOW RESET OPTION
#define __EX_NOREG 16   // DON'T SHOW REGISTERS
#define __EX_WIPEOUT 32 // FULL MEMORY WIPEOUT AND WARMSTART
#define __EX_RPLREGS 64 // SHOW RPL REGISTERS INSTEAD
#define __EX_RPLEXIT 128        // SHOW EXIT OPTION, IT RESUMES EXECUTION AFTER SETTING Exception=EX_EXITRPL
#define __EX_MEMDUMP 256        // SHOW A MEMORY DUMP OF THE ADDRESS PASSED UP AS THE EXCEPTION MESSAGE

/*!
    \brief Throw a user exception
    Cause a user exception to be thrown. It displays the requested message and offer the user several
    options to treat the exception.

    \param message The string that will be displayed.
    \param options One or more of the following constants:
    \li __EX_CONT Display the "Continue" option, allowing the program to continue.
    \li __EX_EXIT Display the "Exit" option, which immediately exits the program.
    \li __EX_WARM Display the "Warmstart" option, which exits the program and causes a
                  restart of the calculator, similar to On-C.
    \li __EX_RESET Display the "Reset" option, which exits the program and then reset the calculator
                   in a way equivalent to a paperclip. It is similar to a Warmstart but also restarts
                   all the ARM hardware.
    \note If the options parameter is passed as 0, the calculator will display the exception message
          and show no options, staying in an infinite loop until the user resets the calc using a
          paperclip. Only use this option if it is not safe to attempt to exit the program.

    \return The function does not return a value, and it may not return at all depending on the
            user's choice to handle the exception. If the user chooses to exit, warmstart or reset
            the program will exit first as if the exit() function was called.
*/
void throw_exception(char *message, unsigned int options);

void throw_dbgexception(char *message, unsigned int options);

// *****************************************************
// **************  IRQ MACHINERY ***********************
// *****************************************************

/*!
    \brief Install an IRQ handler
	Set a routine to service interrupt requests from a specific device. It does not change the
	state of the interrupt controller, so the user needs to manually unmask the interrupt and
	configure the rest of the hardware to generate the proper IRQ request.

    \param service_number Identifies the device that is causing the interrupt. It's a
    					  number from 0 to 31 according to the list below (see Samsung S3C2410X manual).
    \param serv_routine   Address of a service routine. The service routine has no special requirements, other
                          than returning as fast as possible to allow other IRQ's to be processed.
    \note The interrupt service number is as follows:
\liINT_ADC  = [31]
\liINT_RTC  = [30]
\liINT_SPI1 = [29]
\liINT_UART0= [28]
\liINT_IIC  = [27]
\liINT_USBH = [26]
\liINT_USBD = [25]
\liReserved = [24] Not used
\liINT_UART1= [23]
\liINT_SPI0 = [22]
\liINT_SDI  = [21]
\liINT_DMA3 = [20]
\liINT_DMA2 = [19]
\liINT_DMA1 = [18]
\liINT_DMA0 = [17]
\liINT_LCD  = [16]
\liINT_UART2= [15]
\li\liINT_TIMER4=[14]
\liINT_TIMER3=[13]
\liINT_TIMER2=[12] [Used for sound routines, do not use]
\liINT_TIMER1=[11] [Used for timed events, do not use]
\liINT_TIMER0=[10] [Used for system timer, do not use]
\liINT_WDT  = [9]
\liINT_TICK = [8]
\linBATT_FLT= [7]
\liReserved = [6] Not used
\liEINT8_23 = [5] [Used for keyboard routines, do not use]
\liEINT4_7  = [4] [Used for keyboard routines, do not use]
\liEINT3    = [3] [Used for SD Card insertion detect, do not use]
\liEINT2    = [2]
\liEINT1    = [1]
\liEINT0    = [0] [Used for exception handler, do not use]

 \sa __irq_releasehook
*/

void __irq_addhook(int service_number, __interrupt__ serv_routine);

/*!
    \brief Uninstall an IRQ handler
	Removes a service routine that handles interrupt requests from a specific device. It does not change the
	state of the interrupt controller, so the user needs to manually mask the interrupt and
	configure the rest of the hardware to stop generating IRQ requests. If an IRQ is generated
	after this routine is called, it will be serviced by a do-nothing routine.

    \param service_number Identifies the device that is causing the interrupt. It's a
    					  number from 0 to 31 according to the list below (see Samsung S3C2410X manual).
    \note See __irq_addhook for a list of interrupt service numbers

 \sa __irq_addhook
*/

void __irq_releasehook(int service_number);

void __irq_mask(int service_number);
void __irq_unmask(int service_number);
void __irq_clrpending(int service_number);

// Saves __usb_drvstatus
void usb_mutex_lock(void);
void usb_mutex_unlock(void);

/*

KEYBOARD BIT MAP
----------------
This is the bit number in the 64-bit keymatrix.
Bit set means key is pressed.

    A]-+  B]-+  C]-+  D]-+  E]-+  F]-+
    |41|  |42|  |43|  |44|  |45|  |46|
    +--+  +--+  +--+  +--+  +--+  +--+

    G]-+  H]-+  I]-+        UP]+
    |47|  |53|  |54|        |49|
    +--+  +--+  +--+  LF]+  +--+  RT]+
                      |50|  DN]+  |52|
    J]-+  K]-+  L]-+  +--+  |51|  +--+
    |55|  |57|  |58|        +--+
    +--+  +--+  +--+

    M]--+  N]--+  O]--+  P]--+  BKS]+
    | 33|  | 25|  | 17|  | 09|  | 01|
    +---+  +---+  +---+  +---+  +---+

    Q]--+  R]--+  S]--+  T]--+  U]--+
    | 34|  | 26|  | 18|  | 10|  | 02|
    +---+  +---+  +---+  +---+  +---+

    V]--+  W]--+  X]--+  Y]--+  /]--+
    | 35|  | 27|  | 19|  | 11|  | 03|
    +---+  +---+  +---+  +---+  +---+

    AL]-+  7]--+  8]--+  9]--+  *]--+
    | 60|  | 28|  | 20|  | 12|  | 04|
    +---+  +---+  +---+  +---+  +---+

    LS]-+  4]--+  5]--+  6]--+  -]--+
    | 61|  | 29|  | 21|  | 13|  | 05|
    +---+  +---+  +---+  +---+  +---+

    RS]-+  1]--+  2]--+  3]--+  +]--+AL_API
    | 62|  | 30|  | 22|  | 14|  | 06|
    +---+  +---+  +---+  +---+  +---+

    ON]-+  0]--+  .]--+  SP]-+  EN]-+
    | 63|  | 31|  | 23|  | 15|  | 07|
    +---+  +---+  +---+  +---+  +---+

*/

//! Constant for the F1 (A) key
#define KB_A   41
//! Constant for the F2 (B) key
#define KB_B   42
//! Constant for the F3 (C) key
#define KB_C   43
//! Constant for the F4 (D) key
#define KB_D   44
//! Constant for the F5 (E) key
#define KB_E   45
//! Constant for the F6 (F) key
#define KB_F   46
//! Constant for the APPS (G) key
#define KB_G   47
//! Constant for the MODE (H) key
#define KB_H   53
//! Constant for the TOOL (I) key
#define KB_I   54
//! Constant for the VAR (J) key
#define KB_J   55
//! Constant for the STO (K) key
#define KB_K   57
//! Constant for the NXT (L) key
#define KB_L   58
//! Constant for the HIST (M) key
#define KB_M   33
//! Constant for the EVAL (N) key
#define KB_N   25
//! Constant for the ' (O) key
#define KB_O   17
//! Constant for the SYMB (P) key
#define KB_P    9
//! Constant for the Y^X (Q) key
#define KB_Q   34
//! Constant for the Sqrt (R) key
#define KB_R   26
//! Constant for the SIN (S) key
#define KB_S   18
//! Constant for the COS (T) key
#define KB_T   10
//! Constant for the TAN (U) key
#define KB_U    2
//! Constant for the EEX (V) key
#define KB_V   35
//! Constant for the +/- (W) key
#define KB_W   27
//! Constant for the X (X) key
#define KB_X   19
//! Constant for the 1/X (Y) key
#define KB_Y   11
//! Constant for the / (Z) key
#define KB_Z KB_DIV
//! Constant for the 0 key
#define KB_0   31
//! Constant for the 1 key
#define KB_1   30
//! Constant for the 2 key
#define KB_2   22
//! Constant for the 3 key
#define KB_3   14
//! Constant for the 4 key
#define KB_4   29
//! Constant for the 5 key
#define KB_5   21
//! Constant for the 6 key
#define KB_6   13
//! Constant for the 7 key
#define KB_7   28
//! Constant for the 8 key
#define KB_8   20
//! Constant for the 9 key
#define KB_9   12
//! Constant for the backspace key
#define KB_BKS  1
//! Constant for the / (Z) key
#define KB_DIV  3
//! Constant for the * key
#define KB_MUL  4
//! Constant for the + key
#define KB_ADD  6
//! Constant for the - key
#define KB_SUB  5
//! Constant for the . key
#define KB_DOT 23
//! Constant for the SPC key
#define KB_SPC 15
//! Constant for the ENT key
#define KB_ENT  7
//! Constant for the up arrow key
#define KB_UP  49
//! Constant for the down arrow key
#define KB_DN  51
//! Constant for the left arrow key
#define KB_LF  50
//! Constant for the right arrow key
#define KB_RT  52
//! Constant for the ALPHA key
#define KB_ALPHA   60
//! Constant for the left shift key
#define KB_LSHIFT  61
//! Constant for the right shift key
#define KB_RSHIFT  62
//! Constant for the ON key
#define KB_ON  63
//! Constant for the APPS key (prime Only)
#define KB_APP  8
//! Constant for the HOME key (prime Only)
#define KB_HOM 16
//! Constant for the SYMB key (prime Only)
#define KB_SYM 24
//! Constant for the PLOT key (prime Only)
#define KB_PLT 32
//! Constant for the NUM key (prime Only)
#define KB_NUM 48
//! Constant for the HELP key (prime Only)
#define KB_HLP 36
//! Constant for the VIEW key (prime Only)
#define KB_VIE 37
//! Constant for the MENU key (prime Only)
#define KB_MEN 38
//! Constant for the ESC key (prime Only)
#define KB_ESC 39
//! Constant for the CAS key (prime Only)
#define KB_CAS 40

// Unassigned key codes: 56, 59


// MACROS TO CREATE KEYBOARD MASKS
/*!
 * \brief Create a bitmask representing the given key.
 *
 * The bitmask returned by this macro can be compared with the keymatrix result.
 */
#define KB_MASK(a) (((unsigned long long)1)<<(a))

// COMMON KEYBOARD MASKS
// ALL CURSOR KEYS

/*!
 * \brief A bitmask to represent the arrow keys.
 *
 * The keymatrix can be combined with this mask
 * via a bitwise and (&) to eliminate everything except the state of the
 * arrow keys (aka cursor keys).  This is a convenience constant, and its
 * value is equivalent to: \c KB_MASK(KB_UP) \c | \c KB_MASK(KB_DN)
 * \c | \c KB_MASK(KB_LF) \c | \c KB_MASK(KB_RT).
 */
#define KB_CURS ((unsigned long long)0x001e000000000000)

// ALL FUNCTION KEYS (A-F)
/*!
 * \brief A bitmask to represent the function keys.
 *
 * The keymatrix can be combined with this mask
 * via a bitwise and (&) to eliminate everything except the state of the
 * function keys.  This is a convenience constant, and its value is equivalent
 * to: \c KB_MASK(KB_A) \c | \c KB_MASK(KB_B) \c | \c KB_MASK(KB_C)
 * \c | \c KB_MASK(KB_D) \c | \c KB_MASK(KB_E) \c | \c KB_MASK(KB_F).
 */
#define KB_FUNC ((unsigned long long)0x00007e0000000000)

// SHIFT CONSTANTS FOR HIGH-LEVEL KEYBOARD FUNCTIONS

//! Shift constant to use in a combined shiftcode. Shift-Hold.
#define SHIFT_HOLD       0x200
//! Shift constant to use in a combined shiftcode. Hold-On key.
#define SHIFT_ONHOLD     0x40
//! Shift constant to use in a combined shiftcode. Alpha-Hold.
#define SHIFT_ALHOLD     0x400

//! Shift constant to use in a combined shiftcode. Left Shift.
#define SHIFT_LS          0x80
//! Shift constant to use in a combined shiftcode. Right Shift.
#define SHIFT_RS          0x100
//! Shift constant to use in a combined shiftcode. Alpha.
#define SHIFT_ALPHA      0x800

//! Shift constant to use in a combined shiftcode. Hold-Left Shift.
#define SHIFT_LSHOLD      (SHIFT_LS|SHIFT_HOLD)
//! Shift constant to use in a combined shiftcode. Hold-Right Shift.
#define SHIFT_RSHOLD      (SHIFT_RS|SHIFT_HOLD)
//! Shift constant to use in a combined shiftcode. Hold-Alpha.
#define SHIFT_ALPHAHOLD  (SHIFT_ALPHA|SHIFT_ALHOLD)

#define SHIFT_ALPHALOCK  0x1000 // THIS IS NOT FOR THE USER, SYSTEM USE ONLY

//! Shift constant to use in a combined shiftcode. Any Shift or ON.
#define SHIFT_ANY        0xfc0
#define SHIFT_ANYLOCK    0x1fc0 // THIS IS FOR THE SYSTEM ONLY, USED DURING SHIFT MESSAGES

// 18-BIT KEY CODE FOR KEYBOARD HANDLER
//#define KEYCODE(context,shift,key) ((((context)&0x1f)<<13)|(((shift)&SHIFT_ANY))|((key&0x3f)))
//#define KEYCONTEXT(keycode) (((keycode)>>13)&0x1f)
#define OLDKEYSHIFT(keycode) ((keycode<<7)&SHIFT_ANYLOCK)
#define MKOLDSHIFT(keyplane) (((keyplane)&SHIFT_ANYLOCK)>>7)
#define KEYSHIFT(keycode) ((keycode)&SHIFT_ANY)
#define KEYVALUE(keycode) ((keycode)&0x3f)

// KEYMATRIX TYPE DEFINITION
/*!
 * \brief A matrix of simultaneous key states.
 *
 * This data type is a 64-bit integer used to represent the complete state of the keyboard.
 *
 */
typedef unsigned long long keymatrix;

// SCANS THE KEYBOARD AND STORES THE 64-BIT MATRIX
/*!
 * \brief Retrieves the state of the complete keyboard.
 *
 * This function retrieves the state of the entire keyboard in one
 * operation. The result can then be inspected using the various \c KB_*
 * preprocessor macros and constants.
 *
 * \return A ::keymatrix, which will hold the result.
 */
keymatrix keyb_getmatrix();

/*!
 * \brief Tests the current state of a key.
 *
 * This macro detects if a key is pressed.
 *
 * \param key The KB_... constant of a key.
 * \return TRUE if the key is pressed; FALSE otherwise
 *
 */
#define keyb_isKeyPressed(key) (keyb_getkey(0)==key)

/*!
 * \brief Tests whether any key is being pressed on the keyboard.
 * \return TRUE if a key is pressed; FALSE otherwise
 *
 */

#define keyb_isAnyKeyPressed() (keyb_getmatrix()!=0LL)

/*!
 * \brief Tests if the left arrow key is down.
 *
 * This is a convenience macro.
 *
 * \return TRUE if the key is pressed; FALSE otherwise
 */
#define keyb_isLeft() ((keyb_getmatrix()&KB_MASK(KB_LF))!=0)

/*!
 * \brief Tests if the right arrow key is down.
 *
 * This is a convenience macro.
 *
 * \return TRUE if the key is pressed; FALSE otherwise
 */
#define keyb_isRight() ((keyb_getmatrix()&KB_MASK(KB_RT))!=0)

/*!
 * \brief Tests if the up arrow key is down.
 *
 * This is a convenience macro.
 *
 * \return TRUE if the key is pressed; FALSE otherwise
 */
#define keyb_isUp() ((keyb_getmatrix()&KB_MASK(KB_UP))!=0)

/*!
 * \brief Tests if the down arrow key is down.
 *
 * This is a convenience macro.
 *
 * \return TRUE if the key is pressed; FALSE otherwise
 */
#define keyb_isDown() ((keyb_getmatrix()&KB_MASK(KB_DN))!=0)

/*!
 * \brief Tests if the alpha key is down.
 *
 * This is a convenience macro.
 *
 * \return TRUE if the key is pressed; FALSE otherwise
 *
 */
#define keyb_isAlpha() ((keyb_getmatrix()&KB_MASK(KB_ALPHA))!=0)

/*!
 * \brief Tests if the left-shift key is down.
 *
 * This is a convenience macro.
 *
 * \return TRUE if the key is pressed; FALSE otherwise
 *
 */
#define keyb_isLS() ((keyb_getmatrix()&KB_MASK(KB_LSHIFT))!=0)

/*!
 * \brief Tests if the right-shift key is down.
 *
 * This is a convenience macro.
 *
 * \return TRUE if the key is pressed; FALSE otherwise
 *
 */
#define keyb_isRS() ((keyb_getmatrix()&KB_MASK(KB_RSHIFT))!=0)
/*!
 * \brief Tests if the ON key is down.
 *
 * This is a convenience macro.
 *
 * \return TRUE if the key is pressed; FALSE otherwise
 *
 */
#define keyb_isON() ((keyb_getmatrix()&KB_MASK(KB_ON))!=0)

/*!
 * \brief Returns the key constant of the first key pressed.
 *
 * Optionally waits for a non-shift key to be pressed, and then returns a
 * shiftplane specifying which key was pressed and the shift state.
 *
 * If the wait parameter is non-zero, this function does not return until a
 * key has been completely pressed and released (only the key, shift can remain
 * pressed). If multiple keys are pressed simultaneously, the function does
 * not return until all keys have been released; but the return value will be
 * the identifier of the first key.
 * If the wait parameter is zero, the function will wait neither for a key
 * to be pressed or released.
 * The ON key is detected as a normal key, but Shift-ON is not detected.
 *
 * \param wait If 0, return a 0 identifier if no key is pressed;
 *             If non-zero, wait until a key is pressed.
 *
 * \return A shiftcode for the key pressed, or 0 if no key was pressed and
 *         the wait parameter was zero. The shiftcode will be comprised of
 *         a KB_... constant and a combination of the SHIFT_... constants.
 *         Use the KEYCODE() macro to extract the keycode from the shiftcode.
 *         For example,
 *                    int key=keyb_getkey(1);
 *         To check for A regardless of shift state:
 *                    if(KEYCODE(key)==KB_A) ...
 *         To check for LS-A (LS only):
 *                    if(key==(KB_A | SHIFT_LS | SHIFT_LSHOLD)) ...
 *         or         if(KEYCODE(key)==KB_A && SHIFTPLANE(key)==SHIFT_LS|SHIFT_LSHOLD)
 *
 * \note Because this function uses instantaneous keyboard readings, it can only detect
 *       shift-hold planes. Therefore, it always return a combination of (SHIFT_LS|SHIFT_LSHOLD),
 *       (SHIFT_RS|SHIFT_RSHOLD), or (SHIFT_ALPHA|SHIFT_ALPHAHOLD) for the shift plane.
 */
int keyb_getkey(int wait);

//! \brief Keyboard message constant, to be combined with one of the KB_XXX key constants
#define KM_PRESS  0x0000
//! \brief Keyboard message constant, to be combined with one of the KB_XXX key constants
#define KM_REPEAT 0x2000
//! \brief Keyboard message constant, to be combined with one of the KB_XXX key constants
#define KM_LPRESS 0x4000
//! \brief Keyboard message constant, to be combined with one of the KB_XXX key constants
#define KM_LREPEAT (KM_LPRESS|KM_REPEAT)
//! \brief Keyboard message constant, to be combined with one of the KB_XXX key constants
#define KM_KEYDN  0x8000
//! \brief Keyboard message constant, to be combined with one of the KB_XXX key constants
#define KM_KEYUP  0xA000
//! \brief Keyboard message constant, to be combined with one of the KB_XXX key constants
#define KM_SHIFT 0xc000
//! \brief Keyboard message constant, special message from a touch device
#define KM_TOUCH 0xe000


//! \brief Mask to isolate the key shift plane bits
#define KM_SHIFTMASK SHIFT_ANYLOCK
//! \brief Mask to isolate the key value bits
#define KM_KEYMASK  0x003f
//! \brief Mask to isolate the key message bits
#define KM_MSGMASK  0xe000

//! \brief Mask to isolate the X coordinate in a touch message
#define KM_TOUCHXMASK 0xfff
//! \brief Mask to isolate the Y coordinate in a touch message
#define KM_TOUCHYMASK 0xfff0000
//! \brief Mask to isolate the finger ID in a touch message
#define KM_TOUCHFINGERMASK 0x30000000
//! \brief Mask to isolate the touch event in a touch message
#define KM_TOUCHEVENTMASK  0xc0000000

//! \brief Touch event finger down (start touching)
#define KM_FINGERDOWN       0x40000000
//! \brief Touch event finger dragged
#define KM_FINGERMOVE       0x80000000
//! \brief Touch event finger up (stop touching)
#define KM_FINGERUP         0xc0000000


//! \brief Keyboard message queue size (# of messages)
#define KEYB_BUFFER 128
//! \brief Keyboard scanning speed in milliseconds
#define KEYB_SCANSPEED 20

//! \brief Convenience macro to extract message type from a message
#define KM_MESSAGE(a) ( (a) & KM_MSGMASK)
//! \brief Convenience macro to extract pure key code from a message
#define KM_KEY(a) ( (a) & KM_KEYMASK)
//! \brief Convenience macro to extract shifted key code from a message
#define KM_SHIFTEDKEY(a) ( (a) & (KM_KEYMASK|KM_SHIFTMASK))
//! \brief Convenience macro to extract shift plane from a message
#define KM_SHIFTPLANE(a) ( (a) & KM_SHIFTMASK)


//! \brief Convenience macro to extract touch coordinates from a message
#define KM_TOUCHX(a) ( (a) & KM_TOUCHXMASK)
//! \brief Convenience macro to extract touch coordinates from a message
#define KM_TOUCHY(a) (( (a) & KM_TOUCHYMASK)>>16)
//! \brief Convenience macro to extract finger ID from a message
#define KM_TOUCHFINGER(a) (( (a) & KM_TOUCHFINGERMASK)>>28)
//! \brief Convenience macro to extract finger ID from a message
#define KM_TOUCHEVENT(a) ( (a) & KM_TOUCHEVENTMASK)

//! \brief Convenience macro to compose a touch message
#define KM_MAKETOUCHMSG(ev,finger,x,y) ( (ev) | (((finger)<<28)&KM_TOUCHFINGERMASK) \
                                        | ((x)&KM_TOUCHXMASK) | (((y)<<16)&KM_TOUCHYMASK) | KM_TOUCH)

/*!
 * \brief Inserts a key message into the keyboard buffer.
 *
 * Use this function to simulate a keystroke by inserting the proper messages into the keyboard queue.
 * A valid keypress sequence is composed of a KM_KEYDN message, followed by a KM_PRESS and a KM_KEYUP.
 *
 * \param msg A keyboard message, composed of KM_KEYUP, KM_KEYDN or KM_PRESS
 * plus a key value (a KB_XXX constant). KM_PRESS messages can optionally have a SHIFT_XXX shift plane.
 *
 * For example, to simulate pressing the key A in Alpha mode, the sequence of messages
 * can be formed as: (KM_KEYDN | KB_A), then (KM_PRESS | KB_A | SHIFT_ALPHA) and finally
 * (KM_KEYUP | KB_A). If the client program does not require the complete sequence to work,
 * sending only the KM_PRESS message will do the job.
 *
 */
void keyb_postmsg(unsigned int msg);

/*!
 * \brief Get a key message from the keyboard buffer.
 *
 * Use this function to extract key messages from the queue. This function gives access
 * to the most basic keyboard functions. Consider using other higher level keyboard functions
 * for more advanced features, like functions from the stdio module.
 * When a key is pressed, 2 messages are immediately generated: KM_KEYDN and KM_PRESS. Then,
 * if the user keeps the key pressed for a long period of time (see keyb_settiming), an additional
 * KM_PRESS message will be posted. If the user continues to hold the key, an extra KM_PRESS
 * message will be generated at the keyboard repeat rate (see keyb_settiming).
 * When the user releases the key, a KM_KEYUP message will be posted.
 * Notice that Alpha, Left Shift, Right Shift and ON do not generate KM_PRESS messages, only
 * KM_KEYDN/KM_KEYUP pairs.
 * Every time the shift plane changes, a KM_SHIFT message is generated, indicating the new
 * plane. Each KM_PRESS message already carries shift plane information, the KM_SHIFT message
 * is posted to do other tasks like updating the annunciator icons.
 *
 *
 * \return A keyboard message, composed of KM_KEYUP, KM_KEYDN or KM_PRESS
 * plus a key value (a KB_XXX constant). To isolate the key value in the message use
 * the bit mask KM_KEYMASK (like in key=full_msg&KM_KEYMASK). Similarly, to isolate the message type
 * use the bit mask KM_MSGMASK (msg=full_msg&KM_MSGMASK).
 * The KM_PRESS and KM_SHIFT messages include shift planes, that can be isolated
 * using the KM_SHIFTMASK bit mask.
 *
 */
unsigned int keyb_getmsg();

/*!
 * \brief Set all keyboard timing constants.
 *
 * This function defines keyboard timing through 3 parameters.
 *
 *  \param repeat Time in milliseconds between KM_PRESS messages when the user holds
 *                down a key.
 *  \param longpress Time in milliseconds the user needs to hold down a key to generate
 *                   the first KM_LPRESS message and start autorepeating the keypress.
 *  \param debounce Time in milliseconds measured from the moment the user releases a key,
 *                  in which the keyboard handler will ignore additional keypresses of the
 *                  same key
 *
 *
 */
void keyb_settiming(int repeat, int longpress, int debounce);

/*!
 * \brief Activate/deactivate keyboard autorepeat.
 *
 *  \param repeat  Nonzero to activate autorepeat, zero to disable the feature.
 *
 */
void keyb_setrepeat(int repeat);

/*!
 * \brief Activate/deactivate single-alhpa lock mode.
 *
 *  \param single_alpha_lock  Nonzero to lock alpha mode with a single
 *  alpha keypress, zero to require double alpha keypress to lock.
 *
 */
void keyb_setalphalock(int single_alpha_lock);

/*!
 * \brief Manualy set the shift plane.
 *
 *  \param leftshift Nonzero to activate left shift plane.
 *  \param rightshift Nonzero to activate right shift plane.
 *  \param alpha Nonzero to activate alpha shift plane for the next keypress only.
 *  \param alphalock Nonzero to activate alpha mode for all subsequent keypresses.
 *
 * \note This function will wait until all keys have been released prior to changing
 * the shift plane.
 *
 */
void keyb_setshiftplane(int leftshift, int rightshift, int alpha,
        int alphalock);

// OTHER LOWER LEVEL KEYBOARD FUNCTIONS
int keyb_anymsg();
void keyb_flush();
void keyb_flushnowait();
int keyb_wasupdated();
unsigned int keyb_getshiftplane();

#define NUM_EVENTS  5   // NUMBER OF SIMULTANEOUS TIMED EVENTS

// INTERNAL USE ONLY
typedef struct
{
    __interrupt__ eventhandler;
    long long ticks;
    unsigned int delay;
    unsigned int status;        // 1=ACTIVE, 2=AUTORELOAD, 4=PAUSED, NOT FINISHED
} timed_event;

// LOW-LEVEL TIMER STRUCTURE
extern timed_event tmr_events[NUM_EVENTS];

//! Save all timers state to a buffer (13-word)
void tmr_save(unsigned int *tmrbuffer);
//! Restore saved timers state from buffer
void tmr_restore(unsigned int *tmrbuffer);
//! Setup system timers and event scheduler - automatically called at startup
void tmr_setup();

//! Get the frequency of the system timer in ticks per second, normally 100 KHz or 125KHz
tmr_t tmr_getsysfreq();

//! Get system timer count in ticks since program started.
tmr_t tmr_ticks();
//! Do not return until the specified time has passed
void tmr_delayms(int milliseconds);
//! Do not return until the specified time has passed (in microseconds). Accuracy is +/- 10 microseconds.
void tmr_delayus(int microseconds);

//! Calculate elapsed time in milliseconds between before and after (both given in ticks)
int tmr_ticks2ms(tmr_t before, tmr_t after);
//! Calculate elapsed time in microseconds between before and after (both given in ticks)
int tmr_ticks2us(tmr_t before, tmr_t after);
//! Add/subtract an interval in milliseconds to the given time in ticks
tmr_t tmr_addms(tmr_t time, int ms);
//! Add/subtract an interval in microseconds to the given time in ticks
tmr_t tmr_addus(tmr_t time, int us);
//! Wait until the system timer reaches the given time in ticks
void tmr_waituntil(tmr_t time);

//! Macro to convert milliseconds to ticks
#define tmr_ms2ticks(a) tmr_addms((tmr_t)0,(a))

//! Macro to convert milliseconds to ticks
#define tmr_us2ticks(a) tmr_addus((tmr_t)0,(a))

/*!
    \brief Create a timed event.
    Create a new timed event, specifying a callback function that will be called after
    the given time has passed. The autorepeat feature allows the event to be automatically
    rescheduled for another interval of time. Autorepeated events need to be manually removed
    by tmr_eventkill. One-shot events will remove themselves after completion (no need to explicitly
    call tmr_eventkill).

    \param handler  The function that will be called back on every interval
    \param ms       Time interval in milliseconds after which the handler will be called
    \param autorepeat If TRUE, the event will be repeated every 'ms' milliseconds, if FALSE
                      the event will be executed only once after 'ms' milliseconds have passed
                      since the event was created.

    \return An event handler, or -1 if there are no more handles available (see notes).

    \note A maximum of NUM_EVENTS (typically 5) can be created simultaneously.

 \sa tmr_eventkill
*/

HEVENT tmr_eventcreate(__interrupt__ handler, unsigned int ms, int autorepeat);

/*!
    \brief Kill a timed event.
    Stops an autoreloading event.

    \param event The event handler obtained from tmr_eventcreate

 \sa tmr_eventcreate
*/

void tmr_eventkill(HEVENT event);

// BATTERY LEVEL MEASUREMENT API

void bat_setup();
void battery_handler();
// READ THE BATTERY LEVEL AND STORE IT IN __battery
void bat_read();

// VARIABLE WHERE THE BATTERY STATUS IS STORED
extern WORD __battery;

// SYSTEM FONTS
extern const unsigned int Font_5A[];
extern const unsigned int Font_5B[];
extern const unsigned int Font_5C[];
extern const unsigned int Font_6A[];
extern const unsigned int Font_6m[];
extern const unsigned int Font_7A[];
extern const unsigned int Font_8A[];
extern const unsigned int Font_8B[];
extern const unsigned int Font_8C[];
extern const unsigned int Font_8D[];
extern const unsigned int Font_10A[];
extern const unsigned int Font_18[];
extern const unsigned int Font_24[];

// System font only needed on targets that don't have hardware notifications
extern const unsigned int Font_Notifications[];


//const unsigned int System5Font[];
//const unsigned int System6Font[];
//const unsigned int System7Font[];
//const unsigned int MiniFont[];

void DrawText(int x, int y, char *Text, UNIFONT const * Font, int color,
        DRAWSURFACE * drawsurf);
void DrawTextN(int x, int y, char *Text, char *End, UNIFONT const * Font, int color,
        DRAWSURFACE * drawsurf);

void DrawTextBk(int x, int y, char *Text, UNIFONT const * Font, int color,
        int bkcolor, DRAWSURFACE * drawsurf);
void DrawTextBkN(int x, int y, char *Text, char *End, UNIFONT const * Font, int color,
        int bkcolor, DRAWSURFACE * drawsurf);

void DrawTextMono(int x, int y, char *Text, UNIFONT const * Font, int color,
        DRAWSURFACE * drawsurf);
int StringWidth(char *Text, UNIFONT const * Font);
int StringWidthN(char *Text, char *End, UNIFONT const * Font);
char *StringCoordToPointer(char *Text, char *End, UNIFONT const * Font, int *xcoord);

int cpu_getlock(int lockvar, volatile int *lock_ptr);
int cpu_setspeed(int);
void cpu_waitforinterrupt();
void cpu_off();
void cpu_intoff();
void cpu_inton();

// LCD LOW-LEVEL HARDWARE API

extern int __lcd_contrast;
void lcd_sync();
void __lcd_fix();
void lcd_off();
void lcd_on();
void lcd_poweron();
void lcd_setcontrast(int level);
int lcd_setmode(int mode, unsigned int *physbuf);
void lcd_save(unsigned int *buf);
void lcd_restore(unsigned int *buf);
int lcd_scanline();
void lcd_setactivebuffer(int buffer);
int lcd_getactivebuffer();

// BASIC LOW-LEVEL INTERRUPT HANDLERS
void __exception_install();
void __irq_install();
void __keyb_init();

// LOW-LEVEL MEMORY MANAGEMENT
void create_mmu_tables();
void enable_mmu();
void set_stackall();

void memcpyw(void *dest, const void *source, int nwords);
void memmovew(void *dest, const void *source, int nwords);
int stringlen(const char *s);
char *stringcpy(char *t, const char *s);
int safe_stringcpy(char *t, int bsize, const char *s);
void *memcpyb(void *trg, const void *src, int n);
void *memmoveb(void *_dest, const void *_source, int nbytes);
void memsetw(void *dest, int value, int nwords);
void *memsetb(void *dest, int value, int nbytes);

// LOW-LEVEL HARDWARE DRIVERS - POWER
void cpu_off_prepare();
void cpu_off_die();
void cpu_flushwritebuffers();
void cpu_flushTLB();
void cpu_flushicache(void);

// LOW-LEVEL COMMON USB API
void usb_sendcontrolpacket(int packet_type);
void usb_receivecontrolpacket();
int usb_waitforreport();
USB_PACKET *usb_getreport();
void usb_releasereport();

// USER-LEVEL COMMON USB API
int usb_isconnected();
int usb_isconfigured();
int usb_hasdata();
int usb_waitfordata(int nbytes);

int usb_txfileopen(int file_type);
int usb_filewrite(int fileid, BYTEPTR data, int nbytes);
int usb_txfileclose(int fileid);

int usb_rxfileopen();
int usb_rxbytesready(int fileid);
int usb_eof(int fileid);
int usb_fileread(int fileid, BYTEPTR dest, int nbytes);
int usb_rxfileclose(int fileid);
// file_type = 'O','B','W', OR 'D', SEE SPECS
#define usb_filetype(fileid) ((fileid)>>8)

// LOW-LEVEL HARDWARE DRIVERS - KEYBOARD
void __keyb_waitrelease();

// LOW-LEVEL HARDWARE DRIVERS - FLASH MEMORY
void flash_CFIRead(unsigned short *ptr);
void flash_prepareforwriting();
void flash_donewriting();
void ram_startfwupdate();

// LOW-LEVEL MEMORY SUBALLOCATOR FOR FILE SYSTEM
void init_simpalloc();

// LOW-LEVEL DRIVER FOR REAL TIME CLOCK
void rtc_getdatetime(struct date *dt, struct time *tm);
int rtc_setdatetime(struct date dt, struct time tm);
struct date rtc_getdate();
struct time rtc_gettime();
int rtc_settime(struct time tm);
int rtc_setdate(struct date dt);
void rtc_getalarm(struct date *dt, struct time *tm, int *enabled);
int rtc_setalarm(struct date dt, struct time tm, int enabled);
int rtc_chkalrm();
void rtc_setaie(int enabled);
void __rtc_poweron();
void __rtc_poweroff();
void __rtc_reset();

// LOW-LEVEL HARDWARE DRIVERS - USB
void usb_init(int force);
void usb_shutdown();
int usb_isconnected();
int usb_isconfigured();

// HIGHER LEVEL MEMORY MANAGEMENT

WORDPTR *halGrowMemory(BINT zone, WORDPTR * base, BINT newsize);
int halGetFreePages();
int halGetTotalPages();
int halCheckMemoryMap();
int halCountUsedPages(int zone);
int halCheckRplMemory();
void halInitMemoryMap();

// HIGHER LEVEL GLOBAL VARIABLES

extern BINT halFlags;
extern void (*halProcesses[3])(void);
extern HEVENT halBusyEvent, halTimeoutEvent;
extern BINT halLongKeyPending;
extern BINT halKeyMenuSwitch;

// HIGHER LEVEL HAL FUNCTIONS

// CPU AND POWER MANAGEMENT FUNCTIONS
void halSetBusyHandler();
void halInitBusyHandler();
void halEnterPowerOff();

void halPreparePowerOff();
void halWakeUp();
void halCPUSlowMode();
void halCPUFastMode();
void halReset();

// TIMER FUNCTIONS
BINT64 halTicks();

// SOTWARE ALARM FUNCTIONS
void halTriggerAlarm();

// HARDWARE CLOCK AND ALARM FUNCTIONS
struct date halGetSystemDate();
int halSetSystemDate(struct date dt);
struct time halGetSystemTime();
int halSetSystemTime(struct time tm);
void halGetSystemAlarm(struct date *dt, struct time *tm, int *enabled);
int halSetSystemAlarm(struct date dt, struct time tm, int enabled);
void halGetSystemDateTime(struct date *dt, struct time *tm);
void halDisableSystemAlarm();
int halCheckSystemAlarm();

// SCREEN FUNCTIONS
void halInitScreen();
void halSetupTheme(WORDPTR palette);
void halSetNotification(enum halNotification type, int color);
int halGetNotification(enum halNotification type);
void halShowErrorMsg();
void halShowMsg(char *Text);
void halShowMsgN(char *Text, char *End);
void halSetCmdLineHeight(int h);
void halStatusAreaPopup();
void halCancelPopup();
void halUpdateFonts();
void halRedrawAll(DRAWSURFACE * scr);
void halRedrawCmdLine(DRAWSURFACE * scr);
void halUpdateStatus();
void halSetStackHeight(int h);
void halSetFormHeight(int h);
void halSetMenu1Height(int h);
void halSetMenu2Height(int h);

// ERROR REPORTING AND MESSAGES
WORDPTR halGetCommandName(WORDPTR NameObject);

// HIGHER LEVEL UI
BINT halGetContext();
void halSetContext(BINT KeyContext);
void halSetCmdLineMode(BYTE mode);
BYTE halGetCmdLineMode();
void halForceAlphaModeOn();
void halForceAlphaModeOff();

// FORMS, PLOTS AND OTHER SCREENS
void halSwitch2Form();
void halSwitch2Stack();

// OUTER LOOP FLAGS
#define OL_NOEXIT 1     // DON'T POLL EXTERNAL EXIT FUNCTION DURING OUTER POL
#define OL_NOAUTOOFF 2  // DON'T ALLOW AUTO-OFF FEATURE WHILE WAITING
#define OL_NOALARM   4  // DON'T ALLOW EXECUTION OF ALARMS AND TIMERS
#define OL_NOSDFLUSH 8  // DON'T FLUSH SD CACHES AFTER 3 SECONDS
#define OL_LONGPRESS 16 // DETECT LONG PRESS MESSAGE ON ALL KEYS
#define OL_NOCUSTOMKEYS 32      // DON'T DO USER-DEFINED ACTIONS
#define OL_NODEFAULTKEYS 64     // DON'T DO DEFAUL KEY ACTIONS
#define OL_NOCOMMS      128     // DON'T AUTOMATICALLY RECEIVE DATA OVER USB OR SERIAL
#define OL_EXITONERROR  256     // EXIT THE POL IF THERE ARE ANY EXCEPTIONS

// OUTER LOOP
void halOuterLoop(BINT timeoutms, int (*dokey)(WORD), int(*doidle)(WORD),
        BINT flags);
//  IF THIS FUNCTION RETURNS TRUE, TERMINATE THE OUTER LOOP
int halExitOuterLoop();

// KEYBOARD FUNCTIONS
void halInitKeyboard();
BINT halWaitForKey();
BINT halWaitForKeyTimeout(BINT timeoutms);
void halPostKeyboardMessage(WORD keymsg);
int halDoDefaultKey(WORD keymsg);
int halDoCustomKey(WORD keymsg);

// IDLE PROCESSES
void halDeferProcess(void (*function)(void));





// RENDER CACHE EXTERNAL DATA
extern WORDPTR halCacheContents[3 * MAX_RENDERCACHE_ENTRIES];
extern WORD halCacheEntry;


// RENDER

void uiClearRenderCache();
void uiAddCacheEntry(WORDPTR object, WORDPTR bitmap, UNIFONT const ** font);
void uiUpdateOrAddCacheEntry(WORDPTR object, WORDPTR bitmap, UNIFONT const ** font);
WORDPTR uiFindCacheEntry(WORDPTR object, UNIFONT const ** font);
void uiDrawObject(WORDPTR object, DRAWSURFACE * scr, UNIFONT const ** font);
WORDPTR uiRenderObject(WORDPTR object, UNIFONT const ** font);
void uiDrawBitmap(WORDPTR bmp, DRAWSURFACE * scr);

#endif
