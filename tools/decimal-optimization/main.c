#include <stdio.h>
#include <newrpl.h>

#include <time.h>
#include <bindecimal.h>

#define MAX_WORDS 256
#define TOTAL_BREGISTERS 10

#define MACROZeroToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=0; RReg[n].len=1; }
#define MACROOneToRReg(n) { RReg[n].data[0]=1; RReg[n].exp=0; RReg[n].flags=0; RReg[n].len=1; }
#define MACRONANToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=F_NOTANUMBER; RReg[n].len=1; }
#define MACROInfToRReg(n) { RReg[n].data[0]=0; RReg[n].exp=0; RReg[n].flags=F_INFINITY; RReg[n].len=1; }

#define ATAN_TABLES_LEN (2016/8)



WORD BRegData[MAX_WORDS*2*TOTAL_BREGISTERS];

REAL BReg[TOTAL_BREGISTERS];




WORD atanbin_table[MAX_WORDS*CORDIC_TABLESIZE];





int generate_atantable(void)
{

    // INITIALIZE REGISTERS STORAGE

    int k,j;
    BINT extranumber[REAL_REGISTER_STORAGE*2];



    initContext(2016);

    for(k=0;k<REAL_REGISTERS;++k) {
        RReg[k].data=allocRegister();
        newRealFromBINT(&RReg[k],0,0);
    }

    for(k=0;k<TOTAL_BREGISTERS;++k) {
        BReg[k].data=(BINT *)&(BRegData[k*MAX_WORDS*2]);
    }



    REAL one,two_6720,two_6688;

    two_6720.data=extranumber;
    two_6688.data=extranumber+REAL_REGISTER_STORAGE;

    decconst_One(&one);

    newRealFromBINT(&RReg[0],6720,0);
    newRealFromBINT(&RReg[1],2,0);

    Context.precdigits=2024;

    // COMPUTE THE CONSTANT 2^6720
    powReal(&two_6720,&RReg[1],&RReg[0]);

    newRealFromBINT(&RReg[1],2,0);
    newRealFromBINT(&RReg[0],6688,0);
    powReal(&two_6688,&RReg[1],&RReg[0]);

    Context.precdigits=2536;



    printf("#include <stdint.h>\n\n\n// Start of atan(2^-k) table generator!\n\n\n");


    printf("const uint32_t const atan_binary[%d*%d]= {\n",CORDIC_TABLESIZE,CORDIC_TABLEWORDS);


    // START FROM 2^0 = 1
    newRealFromBINT(&RReg[8],1,0);

    int shift_table[CORDIC_TABLESIZE];
    int shift=0;

    for(k=0;k<CORDIC_TABLESIZE;++k)
    {
        // COMPUTE ATAN FROM SERIES X-X^3/3+X^5/5-X^7/7 USING HORNER
        if(k>0) {
        Context.precdigits=2536;
        MACROOneToRReg(2);

        int nterms=1+CORDIC_MAXSYSEXP/k;
        if(nterms<5) nterms=5;
        if(!(nterms&1)) ++nterms;
        //nterms+=50;  // MAKE SURE WE HAVE ENOUGH

        newRealFromBINT(&RReg[1],nterms,0);
        divReal(&RReg[0],&RReg[2],&RReg[1]);

        mulReal(&RReg[4],&RReg[8],&RReg[8]);    // X^2



        for(j=nterms-2;j>0;j-=2)
        {
            mulReal(&RReg[1],&RReg[0],&RReg[4]);    // R(j-1)*x^2
            newRealFromBINT(&RReg[5],j,0);
            divReal(&RReg[3],&RReg[2],&RReg[5]);    // 1/j
            if(j&2) RReg[3].flags|=F_NEGATIVE;
            addReal(&RReg[0],&RReg[1],&RReg[3]);    // R(j)=R(j-1)*x^2 (+/-) 1/j
        }

        mulReal(&RReg[0],&RReg[0],&RReg[8]);    // R(j)*=x

        }

        else {
        trig_atan2(&RReg[8],&one,ANGLERAD);

        // HERE RREG[0] HAS THE RESULT WITH EXTRA DIGITS
        normalize(&RReg[0]);
        }


        Context.precdigits=2536;
        // CONVERT TO AN INTEGER
        mulReal(&RReg[1],&RReg[0],&two_6720);
        roundReal(&RReg[1],&RReg[1],0);
        ipReal(&RReg[2],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

        if(ltReal(&RReg[1],&two_6688)) {
        newRealFromBINT64(&RReg[3],4294967296L,0);
        mul_real(&RReg[1],&two_6720,&RReg[3]);   // INCREASE THE SHIFT by 32 bits
        normalize(&RReg[1]);
        shift+=32;
        copyReal(&two_6720,&RReg[1]);

        // CONVERT TO AN INTEGER AGAIN WITH THE NEW SHIFT
        mulReal(&RReg[1],&RReg[0],&two_6720);
        roundReal(&RReg[1],&RReg[1],0);
        ipReal(&RReg[2],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

        }

        shift_table[k]=shift;

        Context.precdigits=2016;

        // THEN CONVERT TO BINARY FORM

        bIntegerfromReal(&BReg[0],&RReg[2]);

        // CHEAT FOR k==0
        if(k==0) BReg[0].data[0]=2751691783U;   // WE DON'T HAVE SUFFICIENT PRECISION TO COMPUTE THIS FIRST ONE ACCURATELY


        while(BReg[0].data[BReg[0].len-1]==0) --BReg[0].len;

        if(BReg[0].len>CORDIC_TABLEWORDS) {
            printf("Bad number of words!");
            return 0;
        }

        // AND STORE ALL 210 WORDS

        printf("/* atan(2^(-%d)) */\n",k);

        for(j=0;j<BReg[0].len;++j) {
            atanbin_table[k*MAX_WORDS+j]=BReg[0].data[j];
            printf("%uU%c",atanbin_table[k*MAX_WORDS+j],( (j==CORDIC_TABLEWORDS-1)&&(k==CORDIC_TABLESIZE-1))? ' ':',');
            if(j>0 && ((j%21)==20)) printf("\n");
        }
        for(;j<CORDIC_TABLEWORDS;++j) {
            atanbin_table[k*MAX_WORDS+j]=0;
            printf("0U%c",( (j==CORDIC_TABLEWORDS-1)&&(k==CORDIC_TABLESIZE-1))? ' ':',');
            if(j>0 && ((j%21)==20)) printf("\n");
        }


        Context.precdigits=2536;


        // HALF THE ANGLE AND GO AGAIN
        newRealFromBINT(&RReg[1],5,-1);
        mulReal(&RReg[8],&RReg[8],&RReg[1]);

    }

    // BINARY CORDIC TABLE READY
    printf("\n\n};\n\n");


    // OUTPUT SHIFT TABLE
/*
    // NO NEED FOR A SHIFT TABLE, USE shift=INDEX>>5;
    printf("const uint32_t const atan_shift[%d]= {\n",CORDIC_TABLESIZE);

    for(k=0;k<CORDIC_TABLESIZE;++k)
    {
        printf("%d%c%c",shift_table[k],(k==CORDIC_TABLESIZE-1)? ' ':',' , (((k&31)==0)&&(k>0))? '\n':' ');
    }

    printf("\n};\n\n\n");
*/
    return 0;
}




int generate_Ktable(void)
{

    // INITIALIZE REGISTERS STORAGE

    int k,j;
    BINT extranumber[REAL_REGISTER_STORAGE];



    initContext(2016);

    for(k=0;k<REAL_REGISTERS;++k) {
        RReg[k].data=allocRegister();
        newRealFromBINT(&RReg[k],0,0);
    }

    for(k=0;k<TOTAL_BREGISTERS;++k) {
        BReg[k].data=(BINT *)&(BRegData[k*MAX_WORDS*2]);
    }



    REAL one,two_6720;

    two_6720.data=extranumber;

    decconst_One(&one);

    newRealFromBINT(&RReg[0],6720,0);
    newRealFromBINT(&RReg[1],2,0);

    Context.precdigits=2024;

    // COMPUTE THE CONSTANT 2^6720
    powReal(&two_6720,&RReg[1],&RReg[0]);


    Context.precdigits=2016;



    printf("#include <stdint.h>\n\n\n\n// Start of Product(1/sqrt(1+2^-2k)) table generator!\n\n\n");


    printf("const uint32_t const K_binary[%d*%d]= {\n",CORDIC_TABLESIZE,CORDIC_TABLEWORDS);


    // START FROM 1
    newRealFromBINT(&RReg[8],1,0);

    Context.precdigits=2536;

    // 2^(2*K) FOR K=CORDIC_TABLESIZE-1
    newRealFromBINT(&RReg[0],2,0);
    newRealFromBINT(&RReg[1],(CORDIC_TABLESIZE-1)*2,0);
    powReal(&RReg[9],&RReg[0],&RReg[1]);


    for(k=CORDIC_TABLESIZE-1;k>=0;--k)
    {

        Context.precdigits=2536;

        divReal(&RReg[3],&RReg[8],&RReg[9]);    // Prod/2^(2k)
        addReal(&RReg[8],&RReg[8],&RReg[3]);    // Prod+Prod/2^(2k)= Prod*(1+2^(-2k))

        newRealFromBINT(&RReg[0],25,-2);
        mulReal(&RReg[9],&RReg[9],&RReg[0]);    // 0.25*(2)^(2*k) = 2^-2 * (2)^(2*(k-1)) = 2^(2*k-2) = 2^ 2*(k-1)

        // FIND THE SQUARE ROOT OF RReg[8] BY NEWTON RAPHSON

        newRealFromBINT(&RReg[3],5,-1);  // 0.5
        copyReal(&RReg[1],&RReg[8]);
        copyReal(&RReg[0],&RReg[8]);

        do {
        divReal(&RReg[2],&RReg[8],&RReg[1]);    // R/x
        addReal(&RReg[4],&RReg[1],&RReg[2]);    // (x+R/x)
        mulReal(&RReg[0],&RReg[3],&RReg[4]);    // 1/2*(x+R/x)
        swapReal(&RReg[0],&RReg[1]);
        } while(!eqReal(&RReg[0],&RReg[1]));


        // HERE RREG[0] HAS THE RESULT WITH EXTRA DIGITS

        // CONVERT TO AN INTEGER
        divReal(&RReg[1],&two_6720,&RReg[0]);
        roundReal(&RReg[1],&RReg[1],0);
        ipReal(&RReg[2],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

        // THEN CONVERT TO BINARY FORM

        bIntegerfromReal(&BReg[0],&RReg[2]);

        while(BReg[0].data[BReg[0].len-1]==0) --BReg[0].len;

        if(BReg[0].len>CORDIC_TABLEWORDS) {
            printf("Bad number of words!");
            return 0;
        }

        // AND STORE ALL 210 WORDS

        printf("/* Product(1/sqrt(1+2^-2k)) with k=%d...3359 */\n",k);

        for(j=0;j<BReg[0].len;++j) {
            atanbin_table[k*MAX_WORDS+j]=BReg[0].data[j];
            printf("%uU%c",atanbin_table[k*MAX_WORDS+j],( (j==CORDIC_TABLEWORDS-1)&&(k==CORDIC_TABLESIZE-1))? ' ':',');
            if(j>0 && ((j%21)==20)) printf("\n");
        }
        for(;j<CORDIC_TABLEWORDS;++j) {
            atanbin_table[k*MAX_WORDS+j]=0;
            printf("0U%c",( (j==CORDIC_TABLEWORDS-1)&&(k==CORDIC_TABLESIZE-1))? ' ':',');
            if(j>0 && ((j%21)==20)) printf("\n");
        }

    }

    // BINARY CORDIC TABLE READY
    printf("\n\n};\n\n");

    return 0;
}


// GENERATE A TABLE WITH THE CONSTANT 2^K, FOR K=1... 6720 STEP 32

void generate_two_k()
{



    // INITIALIZE REGISTERS STORAGE

    int k,j;
    BINT extranumber[REAL_REGISTER_STORAGE];



    initContext(2016);

    for(k=0;k<REAL_REGISTERS;++k) {
        RReg[k].data=allocRegister();
        newRealFromBINT(&RReg[k],0,0);
    }

    for(k=0;k<TOTAL_BREGISTERS;++k) {
        BReg[k].data=(BINT *)&(BRegData[k*MAX_WORDS*2]);
    }



int two_k_lentable[316];
int two_k_offtable[316];

printf("#include <stdint.h>\n\n\n\n// Start of 2^(32*k) table generator!\n\n\n");


printf("const uint32_t const two_exp_binary[]= {\n");

two_k_offtable[0]=0;

Context.precdigits=4000;


for(k=1;k<=(CORDIC_MAXSYSEXP+CORDIC_MAXSYSEXP/2)/32;++k)
{


    newRealFromBINT(&RReg[2],2,0);
    newRealFromBINT(&RReg[1],k*32,0);
    powReal(&RReg[0],&RReg[2],&RReg[1]);

    // AND STORE ALL 210 WORDS

    printf("\n/* 2^%d */\n",k*32);

    for(j=0;j<RReg[0].len;++j) {
        atanbin_table[k*MAX_WORDS+j]=RReg[0].data[j];
        printf("%uU%c",atanbin_table[k*MAX_WORDS+j],( (j==RReg[0].len-1)&&(k==(CORDIC_MAXSYSEXP+CORDIC_MAXSYSEXP/2)/32))? ' ':',');
        if(j>0 && ((j%21)==20)) printf("\n");
    }
    two_k_lentable[k-1]=RReg[0].len;
    two_k_offtable[k]=two_k_offtable[k-1]+RReg[0].len;

}

// BINARY CORDIC TABLE READY
printf("\n\n};\n\n");

// OUTPUT THE LENGTH AND OFFSET TABLE


printf("const uint32_t const two_exp_offset[]= {\n");

for(j=0;j<(CORDIC_MAXSYSEXP+CORDIC_MAXSYSEXP/2)/32;++j) {
    printf("%uU%c",two_k_offtable[j]|(two_k_lentable[j]<<16), (j==(CORDIC_MAXSYSEXP+CORDIC_MAXSYSEXP/2)/32-1)? ' ':',');
    if(j>0 && ((j%8)==7)) printf("\n");
}

printf("\n\n};\n\n");



}


// GIVEN COS(X) IN RReg[0], RETURN COS(10x) IN RReg[0]
// USES RReg[0] THROUGH RReg[3]

// COS(10X)=512*Z^10-1280*Z^8+1120*Z^6-400*Z^4+50*Z^2-1
// WITH Z=COS(X)

void cos10x()
{
    mulReal(&RReg[1],&RReg[0],&RReg[0]);   // X^2

    RReg[2].flags=0;
    RReg[2].data[0]=512;
    RReg[2].exp=0;
    RReg[2].len=1;          // CONSTANTS FOR POLYNOMIAL

    mulReal(&RReg[0],&RReg[1],&RReg[2]);
    RReg[2].data[0]=1280;
    subReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[0],&RReg[1],&RReg[3]);
    RReg[2].data[0]=1120;
    addReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[0],&RReg[1],&RReg[3]);
    RReg[2].data[0]=400;
    subReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[0],&RReg[1],&RReg[3]);
    RReg[2].data[0]=50;
    addReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[0],&RReg[1],&RReg[3]);
    RReg[2].data[0]=1;
    subReal(&RReg[0],&RReg[0],&RReg[2]);

}

// SAME BUT RECEIVES COS(X)-1 FOR INCREASED PRECISION
void cos10x_prec()
{
    mulReal(&RReg[1],&RReg[0],&RReg[0]);   // X^2
    add_real_mul(&RReg[1],&RReg[1],&RReg[0],2); // 2*X+X^2

    RReg[2].flags=0;
    RReg[2].data[0]=512;
    RReg[2].exp=0;
    RReg[2].len=1;          // CONSTANTS FOR POLYNOMIAL

    mulReal(&RReg[0],&RReg[1],&RReg[2]);
    RReg[2].data[0]=768;
    subReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    addReal(&RReg[0],&RReg[4],&RReg[3]);
    RReg[2].data[0]=1120;
    addReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    addReal(&RReg[0],&RReg[4],&RReg[3]);
    RReg[2].data[0]=400;
    subReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    addReal(&RReg[0],&RReg[4],&RReg[3]);
    RReg[2].data[0]=50;
    addReal(&RReg[3],&RReg[0],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    addReal(&RReg[0],&RReg[4],&RReg[3]);
    RReg[2].data[0]=1;
    subReal(&RReg[0],&RReg[0],&RReg[2]);

}




// COMPUTE COS(X) FOR X<1E-5
// GIVEN X IN RReg[0]
// RETURNS COS(X) IN RReg[0]

void cospower()
{
int k;
int orgexp,digits;
int needdigits=Context.precdigits;
int correction;
int chebyshev_iter=0;

while( (16*chebyshev_iter*chebyshev_iter) < needdigits) chebyshev_iter+=2;

orgexp=RReg[0].exp;
RReg[0].exp=0;
digits=intdigitsReal(&RReg[0]);

correction=orgexp+digits+chebyshev_iter;

if(correction>0) {
    Context.precdigits=needdigits+((3*chebyshev_iter+15)&~7); // GET NECESSARY DIGITS TO PRESERVE VALUE
    RReg[0].exp=-digits-chebyshev_iter;
    if(Context.precdigits<needdigits) Context.precdigits=needdigits+8;
}
else {
    RReg[0].exp=orgexp;
    Context.precdigits+=16;
}


RReg[2].flags=F_NEGATIVE;
RReg[2].exp=0;
RReg[2].len=1;          // FACTORIAL


mulReal(&RReg[3],&RReg[0],&RReg[0]);   // X^2
RReg[0].flags=0;
RReg[0].exp=0;
RReg[0].len=1;
RReg[0].data[0]=1;      // FIRST TERM

RReg[4].flags=0;
RReg[4].exp=0;
RReg[4].len=1;
RReg[4].data[0]=0;      // ACCUMULATOR

// DO AS MANY TERMS AS NEEDED
for(k=2;k<=REAL_PRECISION_MAX/2;k+=2)
{
    mulReal(&RReg[1],&RReg[0],&RReg[3]); // TERM*X^2
    //normalize(&RReg[1]);
    RReg[2].data[0]=k*(k-1);
    divReal(&RReg[0],&RReg[1],&RReg[2]); // NEWTERM= TERM*X^2/(k*(k-1)) = X^K/K!
    // HERE WE HAVE THE NEW TERM OF THE SERIES IN RReg[0]
    addReal(&RReg[5],&RReg[4],&RReg[0]);

    if(eqReal(&RReg[4],&RReg[5])) break;
    swapReal(&RReg[4],&RReg[5]);
}


//printf("COS: total iterations=%d, correction=%d, need_prec=%d, actual prec=%d\n",k,correction,(-RReg[4].exp+7)&~7,Context.precdigits);

if(correction>0) {


RReg[2].data[0]=1;
RReg[2].flags=0;

addReal(&RReg[0],&RReg[4],&RReg[2]);

for(k=0;k<correction;++k) cos10x();
}
else {
    RReg[2].data[0]=1;
    RReg[2].flags=0;

    addReal(&RReg[0],&RReg[4],&RReg[2]);
}


Context.precdigits=needdigits;

}


void trig_cospower(REAL *angle, BINT angmode)
{
    int negsin,negcos,startexp;
    REAL pi,pi2,pi4;
    BINT savedprec;

    negcos=negsin=0;

    savedprec=Context.precdigits;
    Context.precdigits=(2*savedprec+8 > REAL_PRECISION_MAX)? REAL_PRECISION_MAX:(2*savedprec+8);
    if(angle->exp>savedprec) {
        // THIS IS A VERY LARGE ANGLE, NEED TO INCREASE THE PRECISION
        // TO GET AN ACCURATE RESULT ON THE MODULO
        BINT minprec=((savedprec+intdigitsReal(angle))+7)&(~7);
        if(minprec>REAL_PRECISION_MAX) {
            // TODO: ISSUE AN ERROR
            // FOR NOW JUST LEAVE IT WITH PARTIAL LOSS OF PRECISION
            minprec=REAL_PRECISION_MAX;
        }
        Context.precdigits=minprec;
    }

    decconst_PI(&pi);
    decconst_PI_2(&pi2);
    decconst_PI_4(&pi4);

    if(angmode==ANGLERAD) {
        // ANGLE IS IN RADIANS, NO NEED FOR CONVERSION
        copyReal(&RReg[0],angle);
        // GET ANGLE MODULO PI
        divmodReal(&RReg[1],&RReg[0],angle,&pi);
    }
    else {
        REAL convfactor;
        BINT modulo;
        if(angmode==ANGLEDMS) {
            // CONVERT TO DEGREES FIRST, SO THAT THERE'S EXACT VALUES AT 90, ETC.
            trig_convertangle(angle,ANGLEDMS,ANGLEDEG);

            swapReal(&RReg[0],&RReg[7]);
            angle=&RReg[7];

            angmode=ANGLEDEG;   // PLAIN DEGREES FROM NOW ON
        }
        if(angmode==ANGLEDEG) {
            // DEGREES
             decconst_PI_180(&convfactor);
             modulo=180;
        } else {
            // GRADS
             decconst_PI_200(&convfactor);
             modulo=200;
        }

        newRealFromBINT(&RReg[2],modulo,0);

        // GET ANGLE MODULO HALF-TURN
        divmodReal(&RReg[1],&RReg[0],angle,&RReg[2]);

        // CHECK FOR SPECIAL CASES: 1 FULL TURN AND HALF TURN

        if(iszeroReal(&RReg[0])) {
            // EXACT MULTIPLE OF PI, RETURN EXACT VALUES
            MACROOneToRReg(0);
            if(isoddReal(&RReg[1])) RReg[0].flags|=F_NEGATIVE;
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;

            return;
        }
        RReg[2].data[0]>>=1; // 90 OR 100 DEGREES
        RReg[2].flags|=RReg[0].flags&F_NEGATIVE;

        if(eqReal(&RReg[0],&RReg[2])) {
            // EXACT PI/2 OR 3/2PI, RETURN EXACT VALUES
            MACROZeroToRReg(0);
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;


            return;
        }





        // CONVERT TO RADIANS
        mulReal(&RReg[0],&RReg[0],&convfactor);


    }


    // HERE RReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH


    // CHECK FOR SPECIAL CASES

    if(iszeroReal(&RReg[0])) {
        // EXACT MULTIPLE OF PI, IN RADIANS THIS CAN ONLY HAPPEN IF THE ARGUMENT IS ACTUALLY ZERO
        MACROOneToRReg(0);
        if(isoddReal(&RReg[1])) RReg[0].flags|=F_NEGATIVE;
        // RESTORE PREVIOUS PRECISION
        Context.precdigits=savedprec;

        return;
    }

    // IF THE RESULT OF THE DIVISION IS ODD, THEN WE ARE IN THE OTHER HALF OF THE CIRCLE
    if(isoddReal(&RReg[1])) { negcos=negsin=1; }

    if(RReg[0].flags&F_NEGATIVE) { negsin^=1; RReg[0].flags&=~F_NEGATIVE; }

    if(gtReal(&RReg[0],&pi2)) {
        negcos^=1;
        sub_real(&RReg[0],&pi,&RReg[0]);
    }
    /*
    if(gtReal(&RReg[0],&pi4)) {
        swap^=1;
        sub_real(&RReg[0],&pi2,&RReg[0]);
    }
    */
    normalize(&RReg[0]);

    Context.precdigits=savedprec;

    cospower();


    if(negcos) RReg[0].flags|=F_NEGATIVE;

}


// GIVEN SIN(X) IN RReg[0], RETURN SIN(5x) IN RReg[0]
// USES RReg[0] THROUGH RReg[3]

// SIN(5X)=16*Z^5-20*Z^3+5*Z
// Z=SIN(X)


void sin5x()
{
    mulReal(&RReg[1],&RReg[0],&RReg[0]);   // X^2

    RReg[2].flags=0;
    RReg[2].data[0]=16;
    RReg[2].exp=0;
    RReg[2].len=1;          // CONSTANTS FOR POLYNOMIAL

    mulReal(&RReg[4],&RReg[1],&RReg[2]);
    RReg[2].data[0]=20;
    subReal(&RReg[3],&RReg[4],&RReg[2]);

    mulReal(&RReg[4],&RReg[1],&RReg[3]);
    RReg[2].data[0]=5;
    addReal(&RReg[3],&RReg[4],&RReg[2]);

    mulReal(&RReg[4],&RReg[0],&RReg[3]);

    swapReal(&RReg[0],&RReg[4]);

}


// COMPUTE SIN(X)
// GIVEN X IN RReg[0]
// RETURNS SIN(X) IN RReg[0]

void sinpower()
{
int k;
int orgexp,digits;
int needdigits=Context.precdigits;
int correction;
int chebyshev_iter=0;

orgexp=RReg[0].exp;

while( (16*chebyshev_iter*chebyshev_iter) < needdigits) chebyshev_iter+=2;

RReg[0].exp=0;
digits=intdigitsReal(&RReg[0]);

correction=orgexp+digits+chebyshev_iter;

if(correction>0) {
    Context.precdigits=needdigits+((3*chebyshev_iter+15)&~7); // GET NECESSARY DIGITS TO PRESERVE VALUE
    RReg[0].exp=-digits-chebyshev_iter;

    if(Context.precdigits<needdigits) Context.precdigits=needdigits+8;

    // MULTIPLY BY 2^correction
    newRealFromBINT(&RReg[1],1<<correction,0);
    mulReal(&RReg[0],&RReg[0],&RReg[1]);

}
else {
    RReg[0].exp=orgexp;
    Context.precdigits+=16;
}


RReg[2].flags=F_NEGATIVE;
RReg[2].exp=0;
RReg[2].len=1;          // FACTORIAL


mulReal(&RReg[3],&RReg[0],&RReg[0]);   // X^2

//  FIRST TERM IN RReg[0] IS X

copyReal(&RReg[4],&RReg[0]); // ACCUMULATOR

// DO AS MANY TERMS AS NEEDED
for(k=3;k<=REAL_PRECISION_MAX/2;k+=2)
{
    mulReal(&RReg[1],&RReg[0],&RReg[3]); // TERM*X^2
    //normalize(&RReg[1]);
    RReg[2].data[0]=k*(k-1);
    divReal(&RReg[0],&RReg[1],&RReg[2]); // NEWTERM= TERM*X^2/(k*(k-1)) = X^K/K!
    // HERE WE HAVE THE NEW TERM OF THE SERIES IN RReg[0]
    addReal(&RReg[5],&RReg[4],&RReg[0]);

    if(eqReal(&RReg[4],&RReg[5])) break;
    swapReal(&RReg[4],&RReg[5]);
}


//printf("SIN: total iterations=%d, correction=%d, need_prec=%d, actual prec=%d\n",k,correction,(-RReg[4].exp+7)&~7,Context.precdigits);

swapReal(&RReg[0],&RReg[4]);
if(correction>0) {
for(k=0;k<correction;++k) sin5x();
}

Context.precdigits=needdigits;

}


void trig_sinpower(REAL *angle, BINT angmode)
{
    int negsin,negcos,swap,startexp;
    REAL pi,pi2,pi4;
    BINT savedprec;

    negcos=negsin=swap=0;

    savedprec=Context.precdigits;
    Context.precdigits=(2*savedprec+8 > REAL_PRECISION_MAX)? REAL_PRECISION_MAX:(2*savedprec+8);
    if(angle->exp>savedprec) {
        // THIS IS A VERY LARGE ANGLE, NEED TO INCREASE THE PRECISION
        // TO GET AN ACCURATE RESULT ON THE MODULO
        BINT minprec=((savedprec+intdigitsReal(angle))+7)&(~7);
        if(minprec>REAL_PRECISION_MAX) {
            // TODO: ISSUE AN ERROR
            // FOR NOW JUST LEAVE IT WITH PARTIAL LOSS OF PRECISION
            minprec=REAL_PRECISION_MAX;
        }
        Context.precdigits=minprec;
    }

    decconst_PI(&pi);
    decconst_PI_2(&pi2);
    decconst_PI_4(&pi4);

    if(angmode==ANGLERAD) {
        // ANGLE IS IN RADIANS, NO NEED FOR CONVERSION
        copyReal(&RReg[0],angle);
        // GET ANGLE MODULO PI
        divmodReal(&RReg[1],&RReg[0],angle,&pi);
    }
    else {
        REAL convfactor;
        BINT modulo;
        if(angmode==ANGLEDMS) {
            // CONVERT TO DEGREES FIRST, SO THAT THERE'S EXACT VALUES AT 90, ETC.
            trig_convertangle(angle,ANGLEDMS,ANGLEDEG);

            swapReal(&RReg[0],&RReg[7]);
            angle=&RReg[7];

            angmode=ANGLEDEG;   // PLAIN DEGREES FROM NOW ON
        }
        if(angmode==ANGLEDEG) {
            // DEGREES
             decconst_PI_180(&convfactor);
             modulo=180;
        } else {
            // GRADS
             decconst_PI_200(&convfactor);
             modulo=200;
        }

        newRealFromBINT(&RReg[2],modulo,0);

        // GET ANGLE MODULO HALF-TURN
        divmodReal(&RReg[1],&RReg[0],angle,&RReg[2]);

        // CHECK FOR SPECIAL CASES: 1 FULL TURN AND HALF TURN

        if(iszeroReal(&RReg[0])) {
            // EXACT MULTIPLE OF PI, RETURN EXACT VALUES
            MACROZeroToRReg(0);
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;

            return;
        }
        RReg[2].data[0]>>=1; // 90 OR 100 DEGREES
        RReg[2].flags|=RReg[0].flags&F_NEGATIVE;

        if(eqReal(&RReg[0],&RReg[2])) {
            // EXACT PI/2 OR 3/2PI, RETURN EXACT VALUES
            MACROOneToRReg(0);
            if(isoddReal(&RReg[1])) RReg[0].flags|=F_NEGATIVE;
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;


            return;
        }





        // CONVERT TO RADIANS
        mulReal(&RReg[0],&RReg[0],&convfactor);

        normalize(&RReg[0]);

    }


    // HERE RReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH


    // CHECK FOR SPECIAL CASES

    if(iszeroReal(&RReg[0])) {
        // EXACT MULTIPLE OF PI, IN RADIANS THIS CAN ONLY HAPPEN IF THE ARGUMENT IS ACTUALLY ZERO
        MACROZeroToRReg(0);
        // RESTORE PREVIOUS PRECISION
        Context.precdigits=savedprec;

        return;
    }

    // IF THE RESULT OF THE DIVISION IS ODD, THEN WE ARE IN THE OTHER HALF OF THE CIRCLE
    if(isoddReal(&RReg[1])) { negcos=negsin=1; }

    if(RReg[0].flags&F_NEGATIVE) { negsin^=1; RReg[0].flags&=~F_NEGATIVE; }

    if(gtReal(&RReg[0],&pi2)) {
        sub_real(&RReg[0],&pi,&RReg[0]);
        normalize(&RReg[0]);
    }

    if(gtReal(&RReg[0],&pi4)) {
        swap^=1;
        sub_real(&RReg[0],&pi2,&RReg[0]);
        normalize(&RReg[0]);

    }


    Context.precdigits=savedprec;

    if(swap) cospower(); else sinpower();


    if(negsin) RReg[0].flags|=F_NEGATIVE;

}



// COMPUTE SQUARE ROOT OF RReg[0] USING POWER SERIES
// RESULT IS RReg[0]=SQRT(RReg[0]), RReg[1]=1/SQRT(RReg[0]

void psqrt()
{
int orgexp=RReg[0].exp;
int ndigits;


// HANDLE SPECIAL CASE OF 0 (1/SQRT(0) IS INFINITY)
if(iszeroReal(&RReg[0])) {
    RReg[1].data[0]=0;
    RReg[1].exp=0;
    RReg[1].len=1;
    RReg[1].flags=F_INFINITY;

    // RReg[0] IS ALREADY ZERO

    return;

}


RReg[0].exp=0;
ndigits=intdigitsReal(&RReg[0]);

RReg[0].exp=-ndigits;       // MAKE THE NUMBER BE IN THE RANGE 0.1 ... 0.99
int mantexp=ndigits+orgexp;


RReg[1].flags=0;
RReg[1].len=1;

if(mantexp&1) {
    RReg[0].exp++;
    mantexp--;
    // NOW IT'S IN RANGE 1..9.9, SO INITIAL APPROXIMATION SHOULD BE AROUND 0.63
    RReg[1].data[0]=5;
    RReg[1].exp=0;
    if(gtReal(&RReg[0],&RReg[1])) {
        // FIRST APPROXIMATION, START WITH x=0.33
        RReg[1].data[0]=33;
    }
    else {
        // FIRST APPROXIMATION, START WITH x=0.63
        RReg[1].data[0]=63;
    }
    RReg[1].exp=-2;
}
else {
    RReg[1].data[0]=5;
    RReg[1].exp=-1;
    if(gtReal(&RReg[0],&RReg[1])) {
        // FIRST APPROXIMATION, START WITH x=1
        RReg[1].data[0]=1;
    }
    else {
        // FIRST APPROXIMATION, START WITH x=2
        RReg[1].data[0]=2;
    }
    RReg[1].exp=0;
}




RReg[2].exp=0;
RReg[2].flags=0;
RReg[2].len=1;
int savedprec=Context.precdigits;

// Halley's method
Context.precdigits=(Context.precdigits+15)&~7;
//int iters=0;
int goodexp,gooddigits=0;
do {
//    ++iters;
mulReal(&RReg[3],&RReg[1],&RReg[1]);
mulReal(&RReg[4],&RReg[3],&RReg[0]);    // RReg[4]=yn=S*xn^2

RReg[4].flags^=F_NEGATIVE;
RReg[2].data[0]=10;
RReg[2].exp=0;
add_real_mul(&RReg[3],&RReg[2],&RReg[4],3); // (10-3*yn)
normalize(&RReg[3]);
mulReal(&RReg[5],&RReg[4],&RReg[3]);        // -yn*(10-3*yn)
RReg[2].data[0]=15;
addReal(&RReg[4],&RReg[2],&RReg[5]);        // 15-yn*(10-3*yn)
mulReal(&RReg[3],&RReg[4],&RReg[1]);        // xn*(15-yn*(10-3*yn))
RReg[2].data[0]=125;
RReg[2].exp=-3;
mulReal(&RReg[4],&RReg[2],&RReg[3]);        // x(n+1)=0.125*xn*(15-yn*(10-3*yn))
swapReal(&RReg[4],&RReg[1]);
subReal(&RReg[3],&RReg[4],&RReg[1]);
if(RReg[3].len>1) continue;
if(RReg[3].data[0]==0) break;
goodexp=RReg[3].exp+sig_digits(RReg[3].data[0]);
gooddigits=intdigitsReal(&RReg[1])-RReg[1].exp;
gooddigits-=goodexp-RReg[1].exp;

} while(gooddigits<=savedprec);

//printf("iters=%d\n",iters);
// HERE RReg[1] HAS THE RESULT OF 1/SQRT(X)
mulReal(&RReg[0],&RReg[1],&RReg[0]);
// HERE RReg[0]=X*1/SQRT(X) = SQRT(X)

// CORRECT THE NUMBER BY THE EXPONENT
RReg[0].exp+=mantexp/2;
RReg[1].exp-=mantexp/2;




Context.precdigits=savedprec;
}









void trig_tanpower(REAL *angle, BINT angmode)
{
    int negsin,negcos,invert,startexp;
    REAL pi,pi2,pi4;
    BINT savedprec;


    // COMPUTE TANGENT BY COMPUTING FIRST COS(2*X)


    negcos=negsin=invert=0;

    savedprec=Context.precdigits;
    Context.precdigits=(2*savedprec+8 > REAL_PRECISION_MAX)? REAL_PRECISION_MAX:(2*savedprec+8);
    if(angle->exp>savedprec) {
        // THIS IS A VERY LARGE ANGLE, NEED TO INCREASE THE PRECISION
        // TO GET AN ACCURATE RESULT ON THE MODULO
        BINT minprec=((savedprec+intdigitsReal(angle))+7)&(~7);
        if(minprec>REAL_PRECISION_MAX) {
            // TODO: ISSUE AN ERROR
            // FOR NOW JUST LEAVE IT WITH PARTIAL LOSS OF PRECISION
            minprec=REAL_PRECISION_MAX;
        }
        Context.precdigits=minprec;
    }

    decconst_PI(&pi);
    decconst_PI_2(&pi2);
    decconst_PI_4(&pi4);

    addReal(&RReg[1],angle,angle);  // DOUBLE THE ANGLE

    if(angmode==ANGLERAD) {
        // ANGLE IS IN RADIANS, NO NEED FOR CONVERSION
        // GET ANGLE MODULO PI
        divmodReal(&RReg[1],&RReg[0],&RReg[1],&pi);
    }
    else {
        REAL convfactor;
        BINT modulo;
        if(angmode==ANGLEDMS) {
            // CONVERT TO DEGREES FIRST, SO THAT THERE'S EXACT VALUES AT 90, ETC.
            trig_convertangle(&RReg[1],ANGLEDMS,ANGLEDEG);

            swapReal(&RReg[0],&RReg[7]);
            angle=&RReg[7];

            angmode=ANGLEDEG;   // PLAIN DEGREES FROM NOW ON
        }
        if(angmode==ANGLEDEG) {
            // DEGREES
             decconst_PI_180(&convfactor);
             modulo=180;
        } else {
            // GRADS
             decconst_PI_200(&convfactor);
             modulo=200;
        }

        newRealFromBINT(&RReg[2],modulo,0);

        // GET ANGLE MODULO HALF-TURN
        divmodReal(&RReg[1],&RReg[0],angle,&RReg[2]);

        // CHECK FOR SPECIAL CASES: 1 FULL TURN AND HALF TURN

        if(iszeroReal(&RReg[0])) {
            // EXACT MULTIPLE OF PI, RETURN EXACT VALUES
            MACROZeroToRReg(0);
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;

            return;
        }
        RReg[2].data[0]>>=1; // 90 OR 100 DEGREES
        RReg[2].flags|=RReg[0].flags&F_NEGATIVE;

        if(eqReal(&RReg[0],&RReg[2])) {
            // EXACT PI/2 OR 3/2PI, RETURN EXACT VALUES
            MACROInfToRReg(0);
            if(isoddReal(&RReg[1])) RReg[0].flags|=F_NEGATIVE;
            // RESTORE PREVIOUS PRECISION
            Context.precdigits=savedprec;


            return;
        }





        // CONVERT TO RADIANS
        mulReal(&RReg[0],&RReg[0],&convfactor);


    }


    // HERE RReg[0] HAS THE REMAINDER THAT WE NEED TO WORK WITH


    // CHECK FOR SPECIAL CASES

    if(iszeroReal(&RReg[0])) {
        // EXACT MULTIPLE OF PI, IN RADIANS THIS CAN ONLY HAPPEN IF THE ARGUMENT IS ACTUALLY ZERO
        MACROZeroToRReg(0);
        // RESTORE PREVIOUS PRECISION
        Context.precdigits=savedprec;

        return;
    }

    // IF THE RESULT OF THE DIVISION IS ODD, THEN WE ARE IN THE OTHER HALF OF THE CIRCLE
    if(isoddReal(&RReg[1])) { invert=negcos=negsin=1; }

    if(RReg[0].flags&F_NEGATIVE) { negsin^=1; RReg[0].flags&=~F_NEGATIVE; }

    if(gtReal(&RReg[0],&pi2)) {
        negcos^=1;
        sub_real(&RReg[0],&pi,&RReg[0]);

        normalize(&RReg[0]);


    }
    if(gtReal(&RReg[0],&pi4)) {
        sub_real(&RReg[0],&pi2,&RReg[0]);

        normalize(&RReg[0]);

        Context.precdigits=savedprec;

        sinpower();


    } else {

        int startexp=-RReg[0].exp-((RReg[0].len-1)<<3)-sig_digits(RReg[0].data[RReg[0].len-1])+1;


        Context.precdigits=savedprec;

        if(startexp<savedprec) cospower();
        else {
            // VERY SMALL ANGLES, RETURN TAN(X/2)=2/X OR X/2 DEPENDING ON QUADRANT

            Context.precdigits+=16;
            if(invert) {
                RReg[1].data[0]=2;
                RReg[1].exp=0;
                RReg[1].flags=0;
                RReg[1].len=1;
                divReal(&RReg[0],&RReg[1],&RReg[0]);
            }
            else {
                RReg[1].data[0]=5;
                RReg[1].exp=-1;
                RReg[1].flags=0;
                RReg[1].len=1;
                mulReal(&RReg[0],&RReg[1],&RReg[0]);
            }
            Context.precdigits=savedprec;
            //if(negcos) RReg[0].flags|=F_NEGATIVE; // SINCE TAN(X/2)=SIN(X)/(1+COS(X)), ONLY THE SIGN OF SINE IS IMPORTANT
            if(negsin) RReg[0].flags^=F_NEGATIVE;
            return;

        }

    }

    if(negcos) RReg[0].flags|=F_NEGATIVE;


    Context.precdigits+=16;

    MACROOneToRReg(1);

    subReal(&RReg[2],&RReg[1],&RReg[0]);
    addReal(&RReg[3],&RReg[1],&RReg[0]);

    divReal(&RReg[0],&RReg[2],&RReg[3]);

    Context.precdigits=savedprec;

    psqrt();

    if(negsin) RReg[0].flags|=F_NEGATIVE;


}

//extern BINT atan_lltable[9*2016/8];
extern BINT atan_ltable[9*2016/8];



// CALCULATE ATAN(X) USING POWER SERIES
// ARGUMENT IN RReg[0]

void atanpower()
{
    int neg,invert,pass;
    int savedprec=Context.precdigits;
    int k;
    // ARGUMENT REDUCTION
    MACROOneToRReg(1);

    Context.precdigits+=16;

    if(RReg[0].flags&F_NEGATIVE) { neg=1; RReg[0].flags^=F_NEGATIVE; }
    else neg=0;

    // MAKE ANGLE SMALLER TO IMPROVE CONVERGENCE SPEED

    // USE THE IDENTITY ATAN(X) = 1/2 ATAN(2X/(1-X^2))
    // APPLY TWICE AND WORK WITH X/4 OR 4/X

    if(gtReal(&RReg[0],&RReg[1])) {
        invert=1;
        divReal(&RReg[0],&RReg[1],&RReg[0]);    // INVERT THE ARGUMENT
    } else {
        invert=0;
    }

    // HERE 0<=X<=1

    // FIRST PASS - USE IDENTITY ATAN(X)=ATAN(X')+ATAN(DELTA)
    // WITH X' = ANY APPROXIMATION TO X
    // AND DELTA = (X-X')/(1+X*X')
    BINT digit;
    BINT *atan_xtable[ATAN_TABLES_LEN>>5];
    int npasses=Context.precdigits>>8;
    if(npasses<2) npasses=2;

    //printf("passes=%d  ",npasses);
    for(pass=1;pass<=npasses;++pass)
    {

    atan_xtable[pass-1]=0;

    RReg[0].exp+=pass;
    digit=getBINTReal(&RReg[0]);

    RReg[0].exp-=pass;

    RReg[1].data[0]=digit;
    if(digit) {
        RReg[1].exp=-pass;
        atan_xtable[pass-1]=atan_ltable+(9*ATAN_TABLES_LEN)*(pass-1)+ATAN_TABLES_LEN*(digit-1);
        // COMPUTE DELTA

        subReal(&RReg[2],&RReg[0],&RReg[1]);
        mulReal(&RReg[3],&RReg[0],&RReg[1]);
        RReg[1].data[0]=1;
        RReg[1].exp=0;
        addReal(&RReg[4],&RReg[1],&RReg[3]);
        divReal(&RReg[0],&RReg[2],&RReg[4]);    // DELTA = (X-X')/(1+X*X')
    }

    }


    RReg[2].flags=F_NEGATIVE;
    RReg[2].exp=0;
    RReg[2].len=1;          // VARIOUS SMALL CONSTANTS GO HERE


    mulReal(&RReg[3],&RReg[0],&RReg[0]);   // X^2

    //  FIRST TERM IN RReg[0] IS X TO ACCUMULATE

    copyReal(&RReg[4],&RReg[0]); // ACCUMULATOR

    // DO AS MANY TERMS AS NEEDED
    for(k=3;1;k+=2)
    {
        mulReal(&RReg[1],&RReg[0],&RReg[3]); // TERM*X^2

        RReg[2].data[0]=k;
        divReal(&RReg[0],&RReg[1],&RReg[2]); // NEWTERM= TERM*X^2/(K))=X^K/K
        RReg[2].flags^=F_NEGATIVE;
        swapReal(&RReg[0],&RReg[1]);
        // HERE WE HAVE THE NEW TERM OF THE SERIES IN RReg[0]
        addReal(&RReg[5],&RReg[4],&RReg[1]);

        if(eqReal(&RReg[4],&RReg[5])) break;
        swapReal(&RReg[4],&RReg[5]);
    }

    // WE HAVE CONVERGENCE

    //printf("atan iters=%d\n",(k-3)/2);


    REAL pi_2;
    decconst_PI_2(&pi_2);

//  ADD THE KNOWN VALUES AS NEEDED

    BINT needwords=(Context.precdigits+7)/8;
    REAL atan_val;
    atan_val.flags=0;
    for(pass=1;pass<=npasses;++pass)
    {
    if(atan_xtable[pass-1]) {
       atan_val.data=atan_xtable[pass-1]+(ATAN_TABLES_LEN-needwords);
       atan_val.exp=-(needwords*8)-(pass-1);
       atan_val.len=needwords;
       addReal(&RReg[4],&RReg[4],&atan_val);
    }
    }


    if(invert) {
        subReal(&RReg[0],&pi_2,&RReg[4]);
    }
    else swapReal(&RReg[4],&RReg[0]);

    if(neg) RReg[0].flags^=F_NEGATIVE;

    Context.precdigits=savedprec;

}


// GENERATE THE TABLES FOR THE NEW POWER ATAN()
// TABLE HAS 7 GROUPS OF 9 VALUES
// STARTS WITH ATAN(0.1)...ATAN(0.9)
// THEN ATAN(0.01)...ATAN(0.09
// AND SO ON, UP TO 10^-7
void generate_atanlighttables()
{
int j,k;

Context.precdigits=ATAN_TABLES_LEN*8;

for(j=1;j<=7;++j)
{
    for(k=1;k<=9;++k)
    {
    RReg[0].flags=0;
    RReg[0].exp=-j;
    RReg[0].len=1;
    RReg[0].data[0]=k;

    atanpower();

    // NOW STRAIGHTEN THE EXPONENT
    RReg[0].exp+=(ATAN_TABLES_LEN*8)+(j-1);

    roundReal(&RReg[0],&RReg[0],0);

    int_justify(&RReg[0]);
    finalize(&RReg[0]);

    if(RReg[0].len!=ATAN_TABLES_LEN) {
        printf("Bad length!!!");
    }

    printf("\n// ATAN(%d*10^-%d) *****************\n",k,j);

    int s;
    for(s=0;s<RReg[0].len;++s)
    printf("%d%c%c",RReg[0].data[s],((j==7)&&(k==9)&&(s==RReg[0].len-1))? ' ':',' , ((s!=0)&&(s%10==0))? '\n':' ');
    }
}





}


// COMPUTE ASIN(X) WITH X ON RReg[0]
// USING POWER SERIES


void asinhpower()
{

    int k;

    Context.precdigits+=16;

    // POWER SERIES LIMITED TO ABS(X)<1

    RReg[2].flags=F_NEGATIVE;
    RReg[2].exp=0;
    RReg[2].len=1;          // SMALL NUMERATOR CONSTANTS

    RReg[6].flags=0;
    RReg[6].exp=0;
    RReg[6].len=1;          // SMALL DENOMINATOR CONSTANTS


    mulReal(&RReg[3],&RReg[0],&RReg[0]);   // X^2

    //  FIRST TERM IN RReg[0] IS X TO ACCUMULATE

    copyReal(&RReg[4],&RReg[0]); // ACCUMULATOR

    // DO AS MANY TERMS AS NEEDED

    // NEWTERM=OLDTERM*X^2*(k-2)*(k-2)/k/(k-1)

    for(k=3;1;k+=2)
    {
        mulReal(&RReg[1],&RReg[0],&RReg[3]); // TERM*X^2

        RReg[2].data[0]=(k-2)*(k-2);    // LIMITED TO k<10000
        RReg[6].data[0]=k*(k-1);    // LIMITED TO k<10000

        divReal(&RReg[7],&RReg[2],&RReg[6]);
        mulReal(&RReg[0],&RReg[7],&RReg[1]);    // NEWTERM=OLDTERM*X^2*(k-2)*(k-2)/k/(k-1)
        // HERE WE HAVE THE NEW TERM OF THE SERIES IN RReg[0]
        addReal(&RReg[5],&RReg[4],&RReg[0]);

        if(eqReal(&RReg[4],&RReg[5])) break;
        swapReal(&RReg[4],&RReg[5]);

        if(k>9900) { printf("Max iteration limit exceeded: "); break; }
    }

    printf("iters=%d\n",(k-3)/2);

    // CONVERGENCE!

    swapReal(&RReg[0],&RReg[4]);

    Context.precdigits-=16;

}

#define LNTABLE_ENTRIES     16
#define LNTABLE_LEN         (2016/8)
extern BINT ln_ltable[LNTABLE_ENTRIES*LNTABLE_LEN];


void lnpower()
{
    int orgexp,digits,correction;
    int adjustment_const[LNTABLE_ENTRIES];
    int k;
    Context.precdigits+=16;


    MACROOneToRReg(1);

    // SEE HOW CLOSE WE ARE TO 1
    subReal(&RReg[2],&RReg[0],&RReg[1]);




    orgexp=RReg[2].exp;
    RReg[2].exp=0;
    digits=intdigitsReal(&RReg[2]);




    if(orgexp>-digits-LNTABLE_ENTRIES) {
        orgexp=RReg[0].exp;
        RReg[0].exp=0;
        digits=intdigitsReal(&RReg[0]);
        // NEED TO CORRECT THE EXPONENT
        RReg[0].exp=-digits;
        correction=orgexp+digits;

        // START CORRECTION BY CONSTANTS 1+N*10^-(K+1)


        REAL clone;

        k=0;

        // FIRST CONSTANT IS 1.26, THEN (1+11*10^-(K+2))
        adjustment_const[k]=0;
        do {
        cloneReal(&clone,&RReg[0]);
        clone.exp-=2;

        add_real_mul(&RReg[2],&RReg[0],&clone,26);  // A*=( 1+ 0.26 ) = A + A*10^(-2) * 26
        finalize(&RReg[2]);
        if(ltReal(&RReg[2],&RReg[1])) {
        swapReal(&RReg[0],&RReg[2]);
        ++adjustment_const[k];
        } else break;
        } while(1);     //  while A<1.0

        // FROM NOW ON CONSTANTS ARE MORE UNIFORM


        for(k=1;k<LNTABLE_ENTRIES;++k)
        {
            adjustment_const[k]=0;
            do {
            cloneReal(&clone,&RReg[0]);
            clone.exp-=k+2;
            add_real_mul(&RReg[2],&RReg[0],&clone,11);  // A*=( 1+ 11*10^-(K+2) ) = A + A*10^(-K+2) * 11
            finalize(&RReg[2]);
            if(ltReal(&RReg[2],&RReg[1])) {
            swapReal(&RReg[0],&RReg[2]);
            ++adjustment_const[k];
            } else break;
            } while(1);     //  while A<1.0

        }

        // HERE WE HAVE A MUCH CLOSER TO 1.0 BUT STILL A<1.0
        // AND A COUNT OF HOW MANY TIMES EACH MULTIPLIER WAS ADDED


    } else {
        // IS CLOSE ENOUGH TO 1 TO USE THE SERIES DIRECTLY
        for(k=0;k<LNTABLE_ENTRIES;++k) adjustment_const[k]=0;
        correction=0;
    }

    // USE THE POWER SERIES LN(X)=(X-1)-(X-1)^2/2+(X-1)^3/3-...

    subReal(&RReg[2],&RReg[0],&RReg[1]);
    swapReal(&RReg[2],&RReg[0]);


    RReg[2].flags=F_NEGATIVE;
    RReg[2].exp=0;
    RReg[2].len=1;          // SMALL NUMERATOR CONSTANTS

    copyReal(&RReg[3],&RReg[0]);   // (X-1)

    //  FIRST TERM IN RReg[0] IS X-1 TO ACCUMULATE

    copyReal(&RReg[4],&RReg[0]); // ACCUMULATOR

    // DO AS MANY TERMS AS NEEDED

    // NEWTERM=OLDTERM*X/k

    for(k=2;1;k++)
    {
        mulReal(&RReg[1],&RReg[0],&RReg[3]); // TERM*X

        RReg[2].data[0]=k;

        divReal(&RReg[6],&RReg[1],&RReg[2]); // NEWTERM=OLDTERM*X/K
        swapReal(&RReg[0],&RReg[1]);
        // HERE WE HAVE THE NEW TERM OF THE SERIES IN RReg[5], and X^k IN RReg[0]
        addReal(&RReg[5],&RReg[4],&RReg[6]);
        RReg[2].flags^=F_NEGATIVE;
        if(eqReal(&RReg[4],&RReg[5])) break;
        swapReal(&RReg[4],&RReg[5]);

        //if(k>9900) { printf("Max iteration limit exceeded: "); break; }
    }

    //printf("iters=%d\n",k-2);

    // CONVERGENCE!

    // ACCUMULATE CORRECTIONS BY ALL THE DIFFERENT CONSTANTS

    if(correction) {
       REAL ln10;

       decconst_ln10(&ln10);

       RReg[2].flags=0;
       RReg[2].exp=0;
       RReg[2].len=1;
       RReg[2].data[0]=correction;

       mulReal(&RReg[1],&RReg[2],&ln10);
       addReal(&RReg[0],&RReg[1],&RReg[4]);

    } else     swapReal(&RReg[0],&RReg[4]);






    BINT needwords=(Context.precdigits+7)/8;
    // ALL OTHER CONSTANTS FROM TABLE
    REAL lnconstant;

    for(k=0;k<LNTABLE_ENTRIES;++k)
    {
        if(adjustment_const[k]!=0) {
       lnconstant.data=ln_ltable+k*LNTABLE_LEN+LNTABLE_LEN-needwords;
       lnconstant.exp=-needwords*8-k;
       lnconstant.flags=F_NEGATIVE;     // THE CONSTANT IS POSITIVE, ADD THE SIGN HERE SO IT SUBTRACTS THE LN()
       lnconstant.len=needwords;

       add_real_mul(&RReg[1],&RReg[0],&lnconstant,adjustment_const[k]);

       normalize(&RReg[1]);
       swapReal(&RReg[0],&RReg[1]);
        }
    }


    Context.precdigits-=16;

}


void ln1p()
{
int k;
    Context.precdigits+=16;
    // USE THE POWER SERIES LN(X+1)=X-X^2/2+X^3/3-...

    RReg[2].flags=F_NEGATIVE;
    RReg[2].exp=0;
    RReg[2].len=1;          // SMALL NUMERATOR CONSTANTS

    copyReal(&RReg[3],&RReg[0]);   // (X)

    //  FIRST TERM IN RReg[0] IS X TO ACCUMULATE

    copyReal(&RReg[4],&RReg[0]); // ACCUMULATOR

    // DO AS MANY TERMS AS NEEDED

    // NEWTERM=OLDTERM*X/k

    for(k=2;1;k++)
    {
        mulReal(&RReg[1],&RReg[0],&RReg[3]); // TERM*X

        RReg[2].data[0]=k;

        divReal(&RReg[6],&RReg[1],&RReg[2]); // NEWTERM=OLDTERM*X/K
        swapReal(&RReg[0],&RReg[1]);
        // HERE WE HAVE THE NEW TERM OF THE SERIES IN RReg[5], and X^k IN RReg[0]
        addReal(&RReg[5],&RReg[4],&RReg[6]);
        RReg[2].flags^=F_NEGATIVE;
        if(eqReal(&RReg[4],&RReg[5])) break;
        swapReal(&RReg[4],&RReg[5]);

        //if(k>9900) { printf("Max iteration limit exceeded: "); break; }
    }

    //printf("iters=%d\n",k-2);

    // CONVERGENCE!

    swapReal(&RReg[0],&RReg[4]);

    Context.precdigits-=16;
}








// GENERATE THE TABLES FOR THE NEW POWER LN()
// TABLE HAS 16 CONSTANTS
// STARTS WITH LN(1.26)
// THEN LN(1+1.1*10^-K)
// AND SO ON, UP TO 10^-15
void generate_lnlighttables()
{
int j,k;

Context.precdigits=LNTABLE_LEN*8;

    for(k=0;k<LNTABLE_ENTRIES;++k)
    {
    RReg[0].flags=0;
    RReg[0].exp=-(k+2);
    RReg[0].len=1;
    RReg[0].data[0]=(k==0)? 26:11;

    ln1p();

    // NOW STRAIGHTEN THE EXPONENT
    RReg[0].exp+=(LNTABLE_LEN*8)+k;

    roundReal(&RReg[0],&RReg[0],0);

    int_justify(&RReg[0]);
    finalize(&RReg[0]);

    if(RReg[0].len!=LNTABLE_LEN) {
        printf("Bad length!!!");
    }

    printf("\n// LN(1+%d*10^-%d) *****************\n",(k==0)? 26:11,k+2);

    int s;
    for(s=0;s<RReg[0].len;++s)
    printf("%d%c%c",RReg[0].data[s],((k==LNTABLE_ENTRIES-1)&&(s==RReg[0].len-1))? ' ':',' , ((s!=0)&&(s%10==0))? '\n':' ');
    }
}




// COMPUTE EXPONENTIAL OF ANY NUMBER WITHOUT USING TABLES
#define EXP_CONDITIONING    16

void exppower()
{


    // CONCEPT: IF LN(X)=LN(A)+N*LN(10)
    // THEN X=m+N*LN(10) EXP(X) = EXP(m)*10^N
    // WHERE N=IP(X/LN(10)) AND m IS THE REMAINDER

    int k,isneg;
    // RANGE REDUCTION TO +/- LN(10)/2

    // MAKE POSITIVE
    isneg=RReg[0].flags&F_NEGATIVE;
    RReg[0].flags&=~F_NEGATIVE;

    Context.precdigits+=16;
    // GET ANGLE MODULO LN(10)
    REAL ln10,ln10_2;

    decconst_ln10(&ln10);
    decconst_ln10_2(&ln10_2);

    if(iszeroReal(&RReg[0])) {
        // e^0=1
        RReg[0].data[0]=1;
        RReg[0].exp=0;
        RReg[0].flags=RReg[0].flags&F_APPROX;
        RReg[0].len=1;

        Context.precdigits-=16;

        return;

    }

    divmodReal(&RReg[1],&RReg[2],&RReg[0],&ln10);

    // HERE RReg[2] HAS THE REMAINDER THAT WE NEED TO WORK WITH

    // THE QUOTIENT NEEDS TO BE ADDED TO THE EXPONENT, SO IT SHOULD BE +/-30000
    // MAKE SURE THE INTEGER IS ALIGNED AND RIGHT-JUSTIFIED
    if(!inBINTRange(&RReg[1])) {
        // TODO: RAISE OVERFLOW ERROR!
        RReg[0].len=1;
        RReg[0].data[0]=0;
        RReg[0].exp=0;
        if(isneg) RReg[0].flags=0;       // exp(-INF) = 0
            else RReg[0].flags=F_INFINITY;   // exp(INF) = INF
        return;
    }
    BINT quotient=getBINTReal(&RReg[1]);
    if( (quotient>30000) || (quotient<-30000)) {
        // TODO: RAISE OVERFLOW ERROR!
        RReg[0].len=1;
        RReg[0].data[0]=0;
        RReg[0].exp=0;
        if(isneg) RReg[0].flags=0;       // exp(-INF) = 0
            else RReg[0].flags=F_INFINITY;   // exp(INF) = INF
        return;
    }

    if(gtReal(&RReg[2],&ln10_2)) {
        // IS OUTSIDE THE RANGE OF CONVERGENCE
        // SUBTRACT ONE MORE ln(10)
        sub_real(&RReg[0],&RReg[2],&ln10);
        normalize(&RReg[0]);
        // AND ADD IT TO THE EXPONENT CORRECTION
        ++quotient;
    } else swapReal(&RReg[0],&RReg[2]);


    // SPEEDUP WITH POWERS OF 2
    //newRealFromBINT64(&RReg[2],1LL<<EXP_CONDITIONING,0);
    //divReal(&RReg[1],&RReg[0],&RReg[2]);
    //swapReal(&RReg[1],&RReg[0]);

    RReg[0].exp-=EXP_CONDITIONING;  // SPEEDUP THE POWER SERIES WITH POWERS OF TEN

    RReg[2].flags=0;
    RReg[2].exp=0;
    RReg[2].len=1;          // FACTORIAL
    RReg[2].data[0]=1;

    copyReal(&RReg[3],&RReg[0]);   // X

    //  FIRST TERM IN RReg[0] IS X

    addReal(&RReg[4],&RReg[2],&RReg[0]); // ACCUMULATOR STARTS FROM 1+X

    // DO AS MANY TERMS AS NEEDED
    for(k=2;1;k++)
    {
        mulReal(&RReg[1],&RReg[0],&RReg[3]); // TERM*X
        RReg[2].data[0]=k;
        divReal(&RReg[0],&RReg[1],&RReg[2]); // NEWTERM= TERM*X/k = X^K/K!
        // HERE WE HAVE THE NEW TERM OF THE SERIES IN RReg[0]
        addReal(&RReg[5],&RReg[4],&RReg[0]);

        if(eqReal(&RReg[4],&RReg[5])) break;
        swapReal(&RReg[4],&RReg[5]);
    }

    //printf("iters=%d\n",k);

    // CONVERGED!

    // NOW APPLY THE EXPONENT BACK

    RReg[4].exp+=quotient;

    RReg[2].data[0]=1;
    RReg[2].exp=EXP_CONDITIONING;
    powReal(&RReg[1],&RReg[4],&RReg[2]);

    /*
    for(k=0;k<EXP_CONDITIONING;++k)
    {
        mulReal(&RReg[1],&RReg[4],&RReg[4]);
        swapReal(&RReg[1],&RReg[4]);
    }

    swapReal(&RReg[1],&RReg[4]);
    */


    if(isneg) {
        RReg[2].data[0]=1;
        RReg[2].exp=0;
        divReal(&RReg[0],&RReg[2],&RReg[1]);    // EXP(-X)=1/EXP(X)
    } else swapReal(&RReg[1],&RReg[4]);


    Context.precdigits-=16;

    }



int main()
{



    //generate_atantable();
    //generate_Ktable();
    //generate_two_k();


    //return 0;


    // INITIALIZE REGISTERS STORAGE

    int k;
    BINT extranumber[REAL_REGISTER_STORAGE];



    initContext(2000);

    for(k=0;k<REAL_REGISTERS;++k) {
        RReg[k].data=allocRegister();
        newRealFromBINT(&RReg[k],0,0);
    }

    for(k=0;k<TOTAL_BREGISTERS;++k) {
        BReg[k].data=(BINT *)&(BRegData[k*MAX_WORDS*2]);
    }

// ***************************************************************
// CONVERT TABLES FOR 2016 DIGITS INTO SMALLER TABLES AND COMPRESS

//   main_compressor();

//   return 0;

   clock_t start,end;

#define TEST_DIGITS 2000
    Context.precdigits=TEST_DIGITS;


    REAL constpi180;


    decconst_PI_180(&constpi180);



    // ************************************************************************************
    // EXP TEST


    start=clock();




#define TEST_DIGITS 2000
    Context.precdigits=TEST_DIGITS;


//  TEST
    for(k=1;k<100;++k) {


    //MACROOneToRReg(2);
    newRealFromBINT(&RReg[0],k,-1);
    RReg[0].flags=F_NEGATIVE;
    //mulReal(&RReg[0],&RReg[1],&constpi180);

    exppower();
    finalize(&RReg[0]);
    //printf("k=0.%02d --",k);

   }
    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    start=clock();
    //  TEST LN THROUGH POWERS
        for(k=1;k<100;++k) {


        newRealFromBINT(&RReg[0],k,-1);
        RReg[0].flags=F_NEGATIVE;
        hyp_exp(&RReg[0]);

        normalize(&RReg[0]);

        }

        end=clock();

        printf("Done hyp_exp() run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);


    return;


    // ************************************************************************************
    // LN TEST


    start=clock();




#define TEST_DIGITS 2000
    Context.precdigits=TEST_DIGITS;


//  TEST ASINH THROUGH POWERS
    for(k=1;k<100;++k) {


    //MACROOneToRReg(2);
    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    lnpower();
    finalize(&RReg[0]);
    printf("k=0.%02d --",k);

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],k*137,-1);
    mulReal(&RReg[3],&RReg[1],&constpi180);
    MACROOneToRReg(4);

    trig_atan2(&RReg[3],&RReg[4],ANGLERAD);

    normalize(&RReg[0]);

    subReal(&RReg[1],&RReg[0],&RReg[8]);

    int orgexp=RReg[8].exp,digits,diffdigits;

    RReg[8].exp=0;
    digits=intdigitsReal(&RReg[8]);
    RReg[8].exp=orgexp;

    RReg[1].exp-=orgexp;
    diffdigits=intdigitsReal(&RReg[1]);
    ipReal(&RReg[2],&RReg[1],0);
    if(iszeroReal(&RReg[2])) diffdigits=0;
    else printf("k=%d, good digits=%d\n",k,digits-diffdigits);

    */


    }

    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    start=clock();
    //  TEST LN THROUGH POWERS
        for(k=1;k<100;++k) {


        //MACROOneToRReg(2);
        newRealFromBINT(&RReg[1],k,0);
        mulReal(&RReg[0],&RReg[1],&constpi180);

        hyp_ln(&RReg[0]);

        normalize(&RReg[0]);

        }

        end=clock();

        printf("Done hyp_ln() run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    return 0;


    // ************************************************************************************
    // ARCTANGENT TEST


    //generate_atanlighttables();

    //return 0;

    start=clock();




#define TEST_DIGITS 2000
    Context.precdigits=TEST_DIGITS;


//  TEST ATAN THROUGH POWERS
    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[1],k*137,-1);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    atanpower();

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],k*137,-1);
    mulReal(&RReg[3],&RReg[1],&constpi180);
    MACROOneToRReg(4);

    trig_atan2(&RReg[3],&RReg[4],ANGLERAD);

    normalize(&RReg[0]);

    subReal(&RReg[1],&RReg[0],&RReg[8]);

    int orgexp=RReg[8].exp,digits,diffdigits;

    RReg[8].exp=0;
    digits=intdigitsReal(&RReg[8]);
    RReg[8].exp=orgexp;

    RReg[1].exp-=orgexp;
    diffdigits=intdigitsReal(&RReg[1]);
    ipReal(&RReg[2],&RReg[1],0);
    if(iszeroReal(&RReg[2])) diffdigits=0;
    else printf("k=%d, good digits=%d\n",k,digits-diffdigits);

    */


    }

    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    start=clock();

    //  TEST ATAN THROUGH POWERS
        for(k=0;k<100;++k) {


            newRealFromBINT(&RReg[1],k*137,-1);
            mulReal(&RReg[3],&RReg[1],&constpi180);
            MACROOneToRReg(4);

            trig_atan2(&RReg[3],&RReg[4],ANGLERAD);


        finalize(&RReg[0]);


        }

        end=clock();

        printf("Done trig_atan run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);


    return;





    // ************************************************************************************
    // TANGENT TEST


    start=clock();




#define TEST_DIGITS 2000
    Context.precdigits=TEST_DIGITS;

//  TEST TANGENT THROUGH POWERS
    for(k=00;k<100;++k) {


    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    trig_tanpower(&RReg[0],ANGLERAD);
/*
    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    Context.precdigits+=8;
    finalize(&RReg[6]);
    finalize(&RReg[7]);
    divReal(&RReg[7],&RReg[7],&RReg[6]);

    Context.precdigits-=8;

    finalize(&RReg[7]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[7],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in tan(x), k=%d\n",k);
    }
*/

    }

    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);



















    return 0;

//   TEST NEW SQUARE ROOT ALGORITHM

    for(k=0;k<100;++k) {

    newRealFromBINT(&RReg[0],k*11428,-5);

    psqrt();
/*
    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[0],k*11428,-5);
    hyp_sqrt(&RReg[0]);

    finalize(&RReg[0]);
    finalize(&RReg[8]);

    subReal(&RReg[1],&RReg[0],&RReg[8]);

    if(!iszeroReal(&RReg[1])) {
        printf("Error in sqrt(x), k=%d\n",k);
    }
*/
    }

    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    return 0;



    // ************************************************************************************
    // SINE TEST


    start=clock();




#define TEST_DIGITS 2000
    Context.precdigits=TEST_DIGITS;

//  TEST SINE THROUGH POWERS
    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    trig_sinpower(&RReg[0],ANGLERAD);

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[7]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[7],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in sin(x), k=%d\n",k);
    }


    }

    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    start=clock();


//  TEST SINE THROUGH POWERS
    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    trig_sinpower(&RReg[0],ANGLERAD);

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],5*k,-1);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    */

    }

    end=clock();

    printf("Done second run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);


    start=clock();


//  TEST SINE THROUGH POWERS
    for(k=0;k<100;++k) {

    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    //cospower();
    trig_sincos(&RReg[0],ANGLERAD);

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],5*k,-1);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    */

    }

    end=clock();

    printf("Done trig_sincos in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

// END OF SINE TEST
// ********************************************************************************************
return 0;


    // ************************************************************************************
    // COSINE TEST


    start=clock();




#define TEST_DIGITS 100

    Context.precdigits=TEST_DIGITS;

//  TEST COSINE THROUGH POWERS
    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    trig_cospower(&RReg[0],ANGLERAD);
    //trig_sincos(&RReg[0],ANGLERAD);

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }


    }

    end=clock();

    printf("Done first run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

    start=clock();


//  TEST COSINE THROUGH POWERS
    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    cospower();
    //trig_sincos(&RReg[0],ANGLERAD);

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],5*k,-1);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    */

    }

    end=clock();

    printf("Done second run in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);


    start=clock();


//  TEST COSINE THROUGH POWERS
    for(k=0;k<100;++k) {

    newRealFromBINT(&RReg[1],k,0);
    mulReal(&RReg[0],&RReg[1],&constpi180);


    //cospower();
    trig_sincos(&RReg[0],ANGLERAD);

    /*

    swapReal(&RReg[0],&RReg[8]);

    newRealFromBINT(&RReg[1],5*k,-1);
    mulReal(&RReg[0],&RReg[1],&constpi180);

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[8]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    */

    }

    end=clock();

    printf("Done trig_sincos in %.6lf\n",((double)end-(double)start)/CLOCKS_PER_SEC);

// END OF COSINE TEST
// ********************************************************************************************



    return 0;


   return 0;






// TEST OF NEW DROP-IN bintrig_sincos

    for(k=0;k<100;++k) {


    newRealFromBINT(&RReg[0],k*715,-k);     // 0.3

    bintrig_sincos(&RReg[0],ANGLERAD);

    swapReal(&RReg[6],&RReg[8]);
    swapReal(&RReg[7],&RReg[9]);

    finalize(&RReg[8]);
    finalize(&RReg[9]);

    newRealFromBINT(&RReg[0],k*715,-k); // ANGLE 0.3 RADIANS

    trig_sincos(&RReg[0],ANGLERAD);

    finalize(&RReg[6]);
    finalize(&RReg[7]);

    subReal(&RReg[0],&RReg[6],&RReg[8]);
    subReal(&RReg[1],&RReg[7],&RReg[9]);

    if(!iszeroReal(&RReg[0])) {
        printf("Error in cos(x), k=%d\n",k);
    }
    if(!iszeroReal(&RReg[1])) {
        printf("Error in sin(x), k=%d\n",k);
    }

    }
    return 0;



    return 0;

// Original tests

    REAL one,two_6720;

    two_6720.data=extranumber;

    decconst_One(&one);

    newRealFromBINT(&RReg[0],6720,0);
    newRealFromBINT(&RReg[1],2,0);

    Context.precdigits=2024;

    // COMPUTE THE CONSTANT 2^6720
    powReal(&two_6720,&RReg[1],&RReg[0]);


    Context.precdigits=2016;

    // TEST OF A SIMPLE CORDIC LOOP

    newRealFromBINT(&RReg[0],3,-500); // ANGLE 0.3 RADIANS

    mulReal(&RReg[1],&RReg[0],&two_6720);
    roundReal(&RReg[1],&RReg[1],0);
    ipReal(&RReg[1],&RReg[1],1);    // TAKE INTEGER PART AND JUSTIFY THE DIGITS

    bIntegerfromReal(&BReg[0],&RReg[1]);   // CONVERT ANGLE TO BINARY FORM


    bIntegerMul(&BReg[2],&BReg[0],&BReg[0],6720);


    RealfrombInteger(&RReg[0],&BReg[2]);

    divReal(&RReg[0],&RReg[0],&two_6720);


    REAL *ang,*x,*xn,*y,*yn,*tmp;

    REAL table_const;

    ang=&BReg[0];
    x=&BReg[1];
    y=&BReg[2];
    xn=&BReg[3];
    yn=&BReg[4];

    table_const.exp=0;
    table_const.len=CORDIC_TABLEWORDS;
    table_const.flags=0;

    int exp;

    // INITIAL VALUES:
    //X=1 , Y=0

    x->len=CORDIC_TABLEWORDS+1;
    y->len=1;
    x->flags=0;
    y->flags=0;
    for(k=0;k<CORDIC_TABLEWORDS;++k) x->data[k]=y->data[k]=0;
    x->data[k]=1;

    Context.precdigits=2016;

    for(exp=0;exp<CORDIC_TABLESIZE;++exp)
    {
        // Xn=X (-/+) Y>>exp
        // YN=Y (+/-) X>>exp


        if(!(ang->flags&F_NEGATIVE)) {
            y->flags^=F_NEGATIVE;
            bIntegerAddShift(xn,x,y,exp);
            y->flags^=F_NEGATIVE;
            bIntegerAddShift(yn,y,x,exp);
        }
        else {
            bIntegerAddShift(xn,x,y,exp);
            x->flags^=F_NEGATIVE;
            bIntegerAddShift(yn,y,x,exp);
            x->flags^=F_NEGATIVE;
        }

        // READ THE ANGLE FROM THE TABLE

        table_const.data=atan_binary+exp*CORDIC_TABLEWORDS;
        table_const.len=CORDIC_TABLEWORDS;
        while(!table_const.data[table_const.len-1]) --table_const.len;


        if(!(ang->flags&F_NEGATIVE)) {
            table_const.flags^=F_NEGATIVE;
            bIntegerAdd(ang,ang,&table_const);
            table_const.flags^=F_NEGATIVE;
        } else bIntegerAdd(ang,ang,&table_const);

        RealfrombInteger(&RReg[5],ang);
        RealfrombInteger(&RReg[6],&table_const);

        tmp=x; x=xn; xn=tmp;
        tmp=y; y=yn; yn=tmp;

    }

    // FINAL ROTATION BY RESIDUAL ANGLE
    // Xn=X-Y*tan(Ang)=X-Y*Ang
    // Yn=Y+X*tan(Ang)=Y-X*Ang

    bIntegerMul(yn,y,ang,CORDIC_TABLEWORDS*32);
    bIntegerMul(xn,x,ang,CORDIC_TABLEWORDS*32);
    yn->flags^=F_NEGATIVE;
    bIntegerAdd(x,x,yn);
    bIntegerAdd(y,y,xn);



    // RESULTS ARE IN BReg[1] AND BReg[2] FOR x AND y RESPECTIVELY
    // DONE, APPLY CORRECTION FACTOR K= PRODUCT(1/SQRT(1+2^-2K))


    table_const.data=K_binary+(exp-1)*CORDIC_TABLEWORDS;
    table_const.len=CORDIC_TABLEWORDS;
    table_const.flags=0;
    table_const.exp=0;

    bIntegerMul(xn,x,&table_const,CORDIC_TABLEWORDS*32);
    bIntegerMul(yn,y,&table_const,CORDIC_TABLEWORDS*32);

    // FINISHED! RESULTS ARE IN BReg[3] and BReg[4]

    RealfrombInteger(&RReg[3],&BReg[3]);    // COS(ANGLE)
    RealfrombInteger(&RReg[4],&BReg[4]);    // SIN(ANGLE)
    RealfrombInteger(&RReg[5],ang);

    divReal(&RReg[8],&RReg[3],&two_6720);
    divReal(&RReg[9],&RReg[4],&two_6720);

      newRealFromBINT(&RReg[0],3,-500); // ANGLE 0.3 RADIANS

      trig_sincos(&RReg[0],ANGLERAD);

      finalize(&RReg[6]);
      finalize(&RReg[7]);

      subReal(&RReg[0],&RReg[6],&RReg[8]);
      subReal(&RReg[1],&RReg[7],&RReg[9]);
    return 0;
}