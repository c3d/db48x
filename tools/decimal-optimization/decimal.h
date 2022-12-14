/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef DECIMAL_H
#define DECIMAL_H

// CONSTANTS COMMENTED OUT ARE DEFINED BY THE RPL CORE
// TO USE THE LIBRARY AS STAND-ALONE, THEY ARE REQUIRED.

//#define REAL_EXPONENT_MAX    30000

//#define REAL_PRECISION_MAX   2016

#define MAX_EXPONENT    REAL_EXPONENT_MAX
#define MIN_EXPONENT    (-REAL_EXPONENT_MAX-REAL_PRECISION_MAX)

#define MAX_PRECWORDS   (REAL_PRECISION_MAX/8)

//#define REAL_REGISTER_STORAGE    (2*(MAX_PRECWORDS+2))

#define TOTAL_REGISTERS   (REAL_REGISTERS+8)    // FROM 1 TO 32

#define EMPTY_STORAGEBMP (0xffffffffu ^ ((1u<<TOTAL_REGISTERS)-1))

#ifdef NDEBUG
#define PROTECT_WRITE_AREA(ptr,len)
#else
#include <stdio.h>
#include <stdlib.h>
#define PROTECT_WRITE_AREA(ptr,len) { if( ((ptr)<Context.regdata) || ((((WORDPTR)ptr)+(len))>=(WORDPTR)Context.regdata+REAL_REGISTER_STORAGE*TOTAL_REGISTERS)) { printf("PANIC EXIT-BAD WRITE\n"); exit(-1); } }
#endif

/*
typedef int BINT;
typedef long long BINT64;
typedef unsigned int WORD;
typedef unsigned long long UBINT64;
*/
typedef union
{
    UBINT64 w;
    WORD w32[2];
} UWORD;

typedef struct __REAL
{
    WORD flags;
    BINT exp;
    BINT len;
    BINT *data;
} REAL;

typedef struct __NUMBER
{
    WORD flags;
    BINT exp;
    BINT len;
    BINT *data;
    BINT storage[3];
} NUMBER;

typedef struct
{
    WORD flags;
    BINT precdigits;
    WORD alloc_bmp;
    BINT regdata[REAL_REGISTER_STORAGE * TOTAL_REGISTERS];
} CONTEXT;

enum ContextFlags
{
    CTX_OUTOFMEMORY = 1,
    CTX_DIVBYZERO = 2,
    CTX_APPROXIMATED = 4
};

enum RealFlags
{
    F_NEGATIVE = 1,     // SIGN BIT
    F_INFINITY = 2,     // INFINITY
    F_NOTANUMBER = 4,   // NAN
    F_APPROX = 8,       // NUMBER IS GIVEN AS APPROXIMATED,
    // OR THE RESULT OF AN APPROXIMATED OPERATION
    // OR THE RESULT OF AN EXACT OPERATION W/APPROX. OPERANDS
    F_NOTNORMALIZED = 16,       // BIT TO INDICATE IF NUMBER WAS NORMALIZED OR NOT YET

    F_ERROR = 32,       // GENERAL ERROR
    F_OVERFLOW = 64,    // RANGE CHECK HAD TO SHRINK THE EXPONENT
    F_NEGUNDERFLOW = 128,       // RANGE CHECK TURNED TOO SMALL EXPONENT OF NEGATIVE NUMBER INTO ZERO
    F_POSUNDERFLOW = 256,       // RANGE CHECK TURNED TOO SMALL EXPONENT OF POSITIVE NUMBER INTO ZERO
};

enum RealErrors
{
    RERR_EMPTYSTRING = 1,
    RERR_INVALIDCHAR,
    RERR_NODIGITS,
    RERR_EXTRACHARS,
    RERR_BADEXPONENT,
    RERR_BADSIGN,
    RERR_DOUBLEDOT
};

#define F_UNDINFINITY (F_INFINITY|F_NOTANUMBER) // UNDIRECTED COMPLEX INFINITY

// *************************************************************************
// ************* LOW LEVEL API FOR DECIMAL LIBRARY ************************
// *************************************************************************

CONTEXT Context;

void initContext(WORD precision);

BINT *allocRegister();
void freeRegister(BINT * data);

void carry_correct(BINT * start, BINT nwords);

// CHECK THE NUMBER RANGE, CHANGE TO INFINITY OR ZERO AS NEEDED
void checkrange(REAL * number);

// FULLY NORMALIZE A NUMBER
void normalize(REAL * number);

// FASTER ROUNDING ROUTINE WITHOUT SHIFTING
// APPLY ROUNDING IN-PLACE TO THE GIVEN NUMBER OF DIGITS
// OR JUST TRUNCATES THE NUMBER IF truncate IS NON-ZERO

void round_real(REAL * r, int digits, int truncate);

// FULLY NORMALIZE, RANGE CHECK AND ROUND TO SYSTEM PRECISION
void finalize(REAL * number);

// SHIFT AN 8-DIGIT WORD TO THE RIGHT n PLACES (DIVIDE BY 10^n)
// word MUST BE POSITIVE
// n = 0-7

BINT shift_right(BINT word, BINT digits);

// ISOLATE LOW n DIGITS IN A WORD, DISCARD HI DIGITS
BINT lo_digits(BINT word, BINT digits);

// ISOLATE HIGH (8-n) DIGITS IN A WORD, DISCARD LOW DIGITS
// CLEAR THE LOWER n DIGITS IN WORD
BINT hi_digits(BINT word, BINT digits);

// ISOLATE HIGH (8-n) DIGITS IN A WORD, ROUND LOW DIGITS
// CLEAR THE LOWER n DIGITS IN WORD AFTER ROUNDING
BINT hi_digits_rounded(BINT word, BINT digits);

// COUNT NUMBER OF SIGNIFICANT USED DIGITS IN A WORD
// WORD MUST BE NORMALIZED AND >0
BINT sig_digits(BINT word);

// LEFT-JUSTIFY THE DATA OF THE NUMBER
// WITHOUT CHANGING THE VALUE
// ADDS TRAILING ZEROS, SO IT MAY NOT BE POSSIBLE TO DETERMINE
// THE ACTUAL NUMBER OF SIGNIFICANT DIGITS
// AFTER THIS OPERATION

void left_justify(REAL * number);

void add_long(BINT * result, BINT * n1start, BINT nwords);

// SAME BUT SUBTRACTING, NO CARRY CHECKS

void sub_long(BINT * result, BINT * n1start, BINT nwords);

// SINGLE-STEP SHIFT-AND-ACCUMULATE
// MULTIPLIES BY 10^N AND ADDS INTO result
void sub_long_shift(BINT * result, BINT * n1start, BINT nwords, BINT shift);

void zero_words(BINT * ptr, BINT nwords);

void copy_words(BINT * ptr, BINT * source, BINT nwords);

// ADDS 2 REAL NUMBERS AT FULL PRECISION
// NUMBERS SHOULD BE NORMALIZED
void add_real(REAL * r, REAL * a, REAL * b);

// PERFORMS r=a+b*mult, WITH 0<mult<31
// NUMBERS SHOULD BE NORMALIZED
void add_real_mul(REAL * r, REAL * a, REAL * b, BINT mult);
void sub_real_mul(REAL * r, REAL * a, REAL * b, BINT mult);

// ACCUMULATE SMALL INTEGER INTO AN EXISTING REAL (MODIFYING THE ARGUMENT)
void acc_real_int(REAL * result, BINT number, BINT exponent);

// SUBTRACTS 2 REAL NUMBERS AT FULL PRECISION
// NUMBERS SHOULD BE NORMALIZED

void sub_real(REAL * result, REAL * a, REAL * b);

// MULTIPLY 2 REAL NUMBERS AND ACCUMULATE RESULT
// ALL COEFFICIENTS **MUST** BE POSITIVE
// USES NAIVE METHOD WITH THE KARATSUBA TRICK TO GET A 25% SPEEDUP

void mul_real(REAL * r, REAL * a, REAL * b);

// PERFORM KARATSUBA MULTIPLICATION m x m WORDS
// IF m IS ODD, THE SUBDIVISION LEAVES ONE WORD OUT
// SO LAST WORD IS A SINGLE X m MULTIPLICATION

void mul_long_karatsuba(BINT * result, BINT * a, BINT * b, BINT m);

// MULTIPLY 2 REALS AND ACCUMULATE IN result
// USES FULL KARATSUBA METHOD ADAPTED FOR UNBALANCED OPERANDS TOO
// THIS IS THE OUTER CODE SHELL WITH PROPER INITIALIZATION

void mul_real2(REAL * r, REAL * a, REAL * b);

// DIVIDES 2 REALS (OBTAIN DIVISION ONLY, NOT REMAINDER)
// OBTAIN AT LEAST MAXDIGITS SIGNIFICANT FIGURES
// USES LONG DIVISION ALGORITHM

void div_real(REAL * r, REAL * num, REAL * d, int maxdigits);

// DIVIDE A NUMBER USING NEWTON-RAPHSON INVERSION

void div_real_nr(REAL * result, REAL * num, REAL * div);

// *************************************************************************
// ************* HIGH LEVEL API FOR DECIMAL LIBRARY ************************
// *************************************************************************

// INITIALIZE A REAL, OBTAIN STORAGE FOR IT.

void initReal(REAL * a);

// RELEASE MEMORY USED BY REAL

void destroyReal(REAL * a);

// SELECT WORKING PRECISION

void setPrecision(BINT prec);

// GET THE CURRENT PRECISION

BINT getPrecision();

// MAKE A REAL NUMBER FROM AN INTEGER
void newRealFromBINT(REAL * result, BINT number, BINT exp10);

// MAKE A REAL NUMBER FROM A 64-BIT INTEGER
void newRealFromBINT64(REAL * result, BINT64 number, BINT exp10);

// CONVERT TEXT TO A REAL NUMBER
// IT IS UTF8 COMPLIANT, WILL RETURN ERROR IF THERE'S
// ANY INVALID CHARACTERS IN THE text, BETWEEN POINTERS text AND end

void newRealFromText(REAL * result, char *text, char *end, UBINT64 chars);

// COPY CONTENTS OF ONE REAL TO ANOTHER
void copyReal(REAL * dest, REAL * src);

// COPY CONTENTS OF ONE REAL TO ANOTHER WITHOUT MOVING THE DATA
void cloneReal(REAL * dest, REAL * src);

// SWAP THE CONTENTS OF TWO REALS WITHOUT MOVING THE DATA
void swapReal(REAL * n1, REAL * n2);

// ADDITION OF 2 REALS
// DEALS WITH SPECIALS AND FULLY FINALIZE THE ANSWER

void addReal(REAL * result, REAL * a, REAL * b);

// SUBTRACTION OF 2 REALS
// DEALS WITH SPECIALS AND FULLY FINALIZE THE ANSWER

void subReal(REAL * result, REAL * a, REAL * b);

// MULTIPLICATION OF 2 REALS
// DEALS WITH SPECIALS AND FULLY FINALIZES ANSWER

void mulReal(REAL * result, REAL * a, REAL * b);

// DIVIDE 2 REALS, DEAL WITH SPECIALS

void divReal(REAL * result, REAL * a, REAL * b);

// DIVIDE 2 REALS, RETURN INTEGER DIVISION AND REMAINDER, DEAL WITH SPECIALS

void divmodReal(REAL * quotient, REAL * remainder, REAL * a, REAL * b);

// ROUND A REAL NUMBER TO A CERTAIN NUMBER OF DIGITS AFTER DECIMAL DOT
// IF NFIGURES IS NEGATIVE, NFIGURES = TOTAL NUMBER OF SIGNIFICANT DIGITS
// HANDLE SPECIALS

void roundReal(REAL * result, REAL * num, BINT nfigures);

// TRUNCATE A REAL NUMBER TO A CERTAIN NUMBER OF DIGITS AFTER DECIMAL DOT
// IF NFIGURES IS NEGATIVE, NFIGURES = TOTAL NUMBER OF SIGNIFICANT DIGITS
// HANDLE SPECIALS

void truncReal(REAL * result, REAL * num, BINT nfigures);

// RETURN THE INTEGER PART (TRUNCATED)
void ipReal(REAL * result, REAL * num, BINT align);

// RETURN THE FRACTION PART ONLY
void fracReal(REAL * result, REAL * num);

// COMPARISON OPERATORS
BINT ltReal(REAL * a, REAL * b);
BINT gtReal(REAL * a, REAL * b);

BINT lteReal(REAL * a, REAL * b);
BINT gteReal(REAL * a, REAL * b);

BINT eqReal(REAL * a, REAL * b);

// RETURN -1 IF A<B, 0 IF A==B AND 1 IF A>B, -2 IF NAN
// NAN HANDLING IS NOT CONSISTENT WITH OTHER TESTS
// ALL OTHER TESTS FAIL ON NAN, THERE'S NO FAIL CODE IN cmpReal

BINT cmpReal(REAL * a, REAL * b);

// TRUE=1 IF A NUMBER IS ZERO, 0 OTHERWISE
BINT iszeroReal(REAL * n);

// TRUE=1 IF A NUMBER IS INFINITY OR UNDIRECTED INFINITY, 0 OTHERWISE
BINT isinfiniteReal(REAL * n);

// TRUE=1 IF A NUMBER IS UNDIRECTED INFINITY, 0 OTHERWISE
BINT isundinfiniteReal(REAL * n);

// TRUE=1 IF A NUMBER IS INFINITY, UNDIRECTED INFINITY OR NAN, 0 OTHERWISE
BINT isNANorinfiniteReal(REAL * n);

// TRUE=1 IF A NUMBER IS NAN, 0 OTHERWISE
BINT isNANReal(REAL * n);

// RETURN -1 IF NEGATIVE, 1 OTHERWISE
BINT signofReal(REAL * n);

// TRUE IF THE NUMBER HAS NO FRACTIONAL PART (IS AN INTEGER)
BINT isintegerReal(REAL * n);

// TRUE IF THE NUMBER IS NOT DIVISIBLE BY 2
BINT isoddReal(REAL * r);

// GET THE NUMBER OF DIGITS ON THE INTEGER PART OF A NUMBER
BINT intdigitsReal(REAL * r);

// CONVERSION TO/FROM OTHER TYPES

// RETURN TRUE IF THE MAGNITUDE OF THE NUMBER IS WITHIN
// RANGE SO IT CAN BE CONVERTED TO BINT OR BINT64 WITHOUT
// OVERFLOW.
BINT inBINTRange(REAL * n);
BINT inBINT64Range(REAL * n);

// EXTRACT A 32-BIT INTEGER FROM A REAL
// MUST BE WITHIN RANGE
BINT getBINTReal(REAL * n);

// EXTRACT A BINT64 FROM A REAL
// MUST BE WITHIN RANGE
BINT64 getBINT64Real(REAL * n);

// CONVERT A REAL TO FORMATTED TEXT AS FOLLOWS:

// MINIMUM BUFFER SIZE = PRECISION * n+1/n + 10
// n+1/n FOR DIGIT SEPARATOR EVERY n DIGITS
// 10 = '-',...,'.',...,'e','-','00000','.'

// format = BIT FLAGS AS FOLLOWS:
// BITS 0-11 = NUMBER OF DIGITS TO DISPLAY (0-4095)

// BIT 12 = 1 -> DECOMPILE FOR CODE
//        = 0 -> DECOMPILE FOR DISPLAY
// BIT 13 = 1 -> SCIENTIFIC NOTATION N.NNNEXXX
//        = 0 -> NORMAL NNNNNN.NNNN
// BIT 14 = 1 -> ENGINEERING NOTATION, MAKE EXPONENT MULTIPLE OF 3
//        = 0 -> LEAVE EXPONENT AS-IS
// BIT 15 = 1 -> FORCE SIGN +1 INSTEAD OF 1
//        = 0 -> SIGN ONLY IF NEGATIVE
// BIT 16 = 1 -> FORCE SIGN E+1 ON EXPONENT
//        = 0 -> EXPONENT SIGN ONLY IF NEGATIVE
// BIT 17 = 1 -> DO NOT ADD TRAILING DOT FOR APPROX. NUMBERS
//        = 0 -> APPROX. NUMBERS USE TRAILING DOT
// BIT 18 = 1 -> ADD TRAILING ZEROS IF NEEDED TO COMPLETE THE NUMBER OF DIGITS (FIX MODE)
// BIT 19 = 1 -> SUPPRESS ZERO EXPONENTS IN SCI MODE
//        = 0 -> IN SCI MODE, USE ZERO EXPONENTS
// BIT 20 = 1 -> ADD SEPARATOR EVERY x DIGITS FOR FRACTION PART
// BIT 21 = 1 -> ADD SEPARATOR EVERY x DIGITS FOR INTEGER PART
// BIT 22 = 1 -> USE CAPITAL LETTERS
// BITS 23-26 =  PREFERRED EXPONENT FOR ENG NOTATION (0= CHOOSE AUTOMATICALLY)
// BITS 27-30 =  NUMBER OF DIGITS x TO SHOW IN EACH GROUP
// BITS 31    =  SET TO ZERO TO KEEP THE CONSTANTS WITHIN 32-BIT SIGNED NUMBER RANGE

// EXPLANATION:
// BITS 0-11: IN NORMAL FORMAT: MAX. NUMBER OF DECIMAL FIGURES AFTER THE DOT
//            IN SCIENTIFIC OR ENG NOTATION: MAX TOTAL NUMBER OF SIGNIFICANT FIGURES
// BIT 12: DECOMPILE FOR CODE IGNORES THE NUMBER OF DIGITS, IT INCLUDES ALL DIGITS ON THE STRING
//         ALSO IGNORES SEPARATORS EVERY 3 DIGITS, AND IGNORES THE TRAILING DOT DISABLE BIT
// BIT 13: NNNN.MMMM OR N.NNNMMMMEXXX
// BIT 14: ONLY IF BIT 13 IS SET, CHANGE THE EXPONENT TO BE A MULTIPLE OF 3
// BIT 15: DO +1 INSTEAD OF JUST 1
// BIT 16: DO 1E+10 INSTEAD OF 1E10
// BIT 17: DON'T SHOW 3. IF A NUMBER IS APPROXIMATED, ONLY 3
// BIT 18: SHOW NNNN.MMMM0000 WHEN THE NUMBER OF AVAILABLE DIGITS IS LESS THAN THE REQUESTED NUMBER
// BIT 19: DON'T SHOW ZERO EXPONENT 2.5 INSTEAD OF 2.5E0 IN SCI AND ENG, AND WHEN PREFERRED EXPONENT MODE, SUPRESS ALL EXPONENTS
// BIT 20: SHOW NNNNNNN.MMM MMM MMM
// BIT 21: SHOW NNN,NNN,NNN,NNN.MMMMMM
// BIT 22: SHOW 1E10 OR 1e10
// BIT 23-26 = DISPLAYS A NUMBER WITH A PREFERRED EXPONENT EXP=(n-7)*3, n=0 MEANS DON'T FORCE EXPONENT
// BITS 27-30: NUMBER OF DIGITS IN A GROUP 0-15

// THE SEPARATORS AND DECIMAL ARE GIVEN IN THE chars ARGUMENT AS 16-BIT VALUES (ONLY UNICODE CP 0-0xFFFF ARE ALLOWED)
// LSB = DECIMAL DOT (USUALLY DOT OR COMMA)
// 2ND = THOUSAND SEPARATOR (SI RECOMMENDS THIN SPACE 0x2009)
// 3RD = DECIMAL DIGIT SEPARATOR (SI RECOMMENDS THIN SPACE 0x2009)
// MSB = ARGUMENT SEPARATOR (USUALLY COMMA OR SEMICOLON)

// ALL FOUR CHARACTERS ARE PACKED IN A 32-BIT WORD
enum FORMAT_BITS
{
    FMT_NUMDIGITS = 0x00000fff,
    FMT_CODE = 0x00001000,
    FMT_SCI = 0x00002000,
    FMT_ENG = 0x00004000,
    FMT_FORCESIGN = 0x00008000,
    FMT_EXPSIGN = 0x00010000,
    FMT_NOTRAILDOT = 0x00020000,
    FMT_TRAILINGZEROS = 0x00040000,
    FMT_SUPRESSEXP = 0x00080000,
    FMT_FRACSEPARATOR = 0x00100000,
    FMT_NUMSEPARATOR = 0x00200000,
    FMT_USECAPITALS = 0x00400000,
    FMT_PREFEXPMSK = 0x07800000,
    FMT_GROUPDIGITSMSK = 0x78000000,

};

#define FMT_GROUPDIGITS(a) (((a)&0xf)<<27)
#define SEP_SPACING(a) (((a)>>27)&0xf)
#define EXP_LETTER(a) (((a)&FMT_USECAPITALS)? 'E':'e')
#define FMT_PREFEREXPONENT(exp) (((((exp)/3)+8)&0xf)<<23)
#define FMT_PREFEREXPRAW(a) (((a)&0xf)<<23)
#define PREFERRED_EXP(a) (((int)(((a)>>23)&0xf)-8)*3)
#define PREFERRED_EXPRAW(a) ((int)(((a)>>23)&0xf))

// MACROS TO BE USED WITH THE LOCALE CHARACTERS

#define DECIMAL_DOT(a) ((WORD)((a)&0xffff))
#define THOUSAND_SEP(a) ((WORD)(((a)>>16)&0xffff))
#define FRAC_SEP(a) ((WORD)(((a)>>32)&0xffff))
#define ARG_SEP(a) ((WORD)(((a)>>48)&0xffff))
#define MAKELOCALE(dec,thousand,frac,arg) ((((UBINT64)((dec)&0xffff))<<0)|(((UBINT64)((thousand)&0xffff))<<16)|(((UBINT64)((frac)&0xffff))<<32)|(((UBINT64)((arg)&0xffff))<<48))

// DEFAULT LOCALE CHARACTERS (FOUR UNICODE CODE POINTS)
#define SYSTEM_DEFAULT_LOCALE MAKELOCALE('.',THIN_SPACE,THIN_SPACE,',')

char *formatReal(REAL * number, char *buffer, BINT format, UBINT64 chars);

// RETURNS AN ESTIMATED SIZE OF BUFFER GUARANTEED TO HOLD THE TEXT
// GENERATED BY formatReal()
BINT formatlengthReal(REAL * number, BINT format, UBINT64 locale);

// *************************************************************************
// **************************** END DECIMAL LIBRARY ************************
// *************************************************************************

#endif // DECIMAL_H
