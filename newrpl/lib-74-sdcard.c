/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// ******************************************
// INCLUDE THIS FOR DEBUG ONLY - REMOVE WHEN DONE
#include "../firmware/sys/target_50g/fsystem/fsyspriv.h"
// ******************************************



// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  74

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(SDRESET,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDSETPART,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDSTO,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDRCL,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDCHDIR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDMKDIR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDPGDIR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDOPENRD,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDOPENWR,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDOPENAPP,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDOPENMOD,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDCLOSE,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDREADTEXT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDWRITETEXT,MKTOKENINFO(11,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDREADLINE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

#define ERROR_LIST \
    ERR(UNKNOWNFSERROR,0), \
    ERR(ENDOFFILE,1), \
    ERR(BADFILENAME,2), \
    ERR(BADVOLUME,3), \
    ERR(FILENOTFOUND,4), \
    ERR(CANTWRITE,5), \
    ERR(NOCARD,6), \
    ERR(CARDCHANGED,7), \
    ERR(MAXFILES,8), \
    ERR(ALREADYOPEN,9), \
    ERR(DISKFULL,10), \
    ERR(ALREADYEXISTS,11), \
    ERR(INVALIDHANDLE,12), \
    ERR(IDENTORPATHEXPECTED,13), \
    ERR(NOTANRPLFILE,14)




// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib74_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib74_menu,
    0
};

// CONVERT FILE SYSTEM ERROR MESSAGE INTO THIS LIBRARY ERRORS
BINT rplFSError2Error(BINT err)
{
    switch(err) {
    case FS_EOF:	    			// END OF FILE
        return ERR_ENDOFFILE;
    case FS_BADNAME:   			// INVALID FILE NAME
        return ERR_BADFILENAME;
    case FS_BADVOLUME: 			// INVALID DRIVE
        return ERR_BADVOLUME;
    case FS_NOTFOUND:  			// FILE NOT FOUND
        return ERR_FILENOTFOUND;
    case FS_CANTWRITE: 			// WRITE FAILED
        return ERR_CANTWRITE;
    case FS_NOCARD:    			// NO CARD INSERTED
        return ERR_NOCARD;
    case FS_CHANGED:    		// CARD HAS CHANGED
        return ERR_CARDCHANGED;
    case FS_MAXFILES:  			// MAXIMUM NUMBER OF FILES OPEN WAS EXCEEDED
        return ERR_MAXFILES;
    case FS_USED:      			// FILE/DIRECTORY IS BEING USED
        return ERR_ALREADYOPEN;
    case FS_DISKFULL:  		// DISK IS FULL
        return ERR_DISKFULL;
    case FS_EXIST:     		// FILE ALREADY EXISTS
        return ERR_ALREADYEXISTS;
    case FS_INVHANDLE: 		// HANDLE IS NOT VALID
        return ERR_INVALIDHANDLE;
    default:
    case FS_ERROR:      		    // UNKNOWN ERROR (OR FUNCTION DOESN'T CARE)
        return ERR_UNKNOWNFSERROR;
    }
}



void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case SDRESET:
    {
        // REINIT FILE SYSTEM
        int error=FSRestart();
        if(error!=FS_OK) {
            rplNewBINTPush((BINT64)FSystem.CurrentVolume,HEXBINT);
            rplError(rplFSError2Error(error));
        }
        return;
    }

    case SDSETPART:
    {
        // SET CURRENT SD CARD PARTITION
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT64 partnum=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        BINT ismounted=FSVolumeInserted(partnum);

        if(ismounted==FS_OK) {
            FSSetCurrentVolume(partnum);
       }
        else  {
         rplError(rplFSError2Error(ismounted));
        }

        rplDropData(1);

        return;

    }

    case SDSTO:
    {
        // STORE AN OBJECT DIRECTLY INTO A FILE
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // TODO: MAKE A PATH BY APPENDING ALL STRINGS/IDENTS



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO OPEN THE FILE

        FS_FILE *objfile;
        const char const *fileprolog="NRPL";
        int err;
        err=FSOpen((char *)(RReg[0].data),FSMODE_WRITE,&objfile);
        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
            }
        BINT objlen=rplObjSize(rplPeekData(2));
        if(FSWrite((char *)fileprolog,4,objfile)!=4) {
            FSClose(objfile);
            rplError(ERR_CANTWRITE);
            return;
        }
        err=FSWrite((char *)rplPeekData(2),objlen*sizeof(WORD),objfile);
        if(err!=objlen*sizeof(WORD)) {
            FSClose(objfile);
            rplError(ERR_CANTWRITE);
            return;
        }
        FSClose(objfile);
        rplDropData(2);

        return;

    }

    case SDRCL:
    {
        // RCL AN OBJECT DIRECTLY FROM A FILE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // TODO: MAKE A PATH BY APPENDING ALL STRINGS/IDENTS



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO OPEN THE FILE

        FS_FILE *objfile;
        const char const *fileprolog="NRPL";
        int err;
        err=FSOpen((char *)(RReg[0].data),FSMODE_READ,&objfile);
        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
            }
        BINT objlen=FSFileLength(objfile);
        if(FSRead((char *)(RReg[0].data),4,objfile)!=4) {
            FSClose(objfile);
            rplError(ERR_NOTANRPLFILE);
            return;
        }
        if((WORD)RReg[0].data[0]!=*((WORD *)fileprolog)) {
            FSClose(objfile);
            rplError(ERR_NOTANRPLFILE);
            return;
        }

        // DROP THE NAME FROM THE STACK
        rplDropData(1);

        objlen-=4;
        while(objlen>=4) {
            if(FSRead((char *)(RReg[0].data),4,objfile)!=4) {
                FSClose(objfile);
                rplError(ERR_NOTANRPLFILE);
                return;
            }
            BINT objsize=rplObjSize((WORDPTR)RReg[0].data);
            if(objsize*sizeof(WORD)<objlen) {
                FSClose(objfile);
                rplError(ERR_NOTANRPLFILE);
                return;
            }
            WORDPTR newobj=rplAllocTempOb(objsize-1);
            if(!newobj) {
                FSClose(objfile);
                return;
            }
            newobj[0]=(WORD)RReg[0].data[0];
            if(FSRead((char *)(newobj+1),(objsize-1)*sizeof(WORD),objfile)!=(objsize-1)*sizeof(WORD)) {
                FSClose(objfile);
                rplError(ERR_NOTANRPLFILE);
                return;
            }

            // OBJECT WAS READ SUCCESSFULLY
            // TODO: ASK THE LIBRARY TO VERIFY IF THE OBJECT IS VALID

            rplPushData(newobj);


            objlen-=objsize*sizeof(WORD);
        }

        // DONE READING ALL OBJECTS FROM THE FILE
        FSClose(objfile);

        // IF THERE ARE MULTIPLE OBJECTS, SHOULDN'T WE RETURN THE NUMBER?

        return;

    }

    case SDCHDIR:
    {
        // CHANGE CURRENT DIRECTORY
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // TODO: MAKE A PATH BY APPENDING ALL STRINGS/IDENTS



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO CHANGE CURRENT DIR

        BINT err=FSChdir((char *)path);

        if(err!=FS_OK) {
            rplFSError2Error(err);
        }
        else rplDropData(1);

        return;

    }


    case SDMKDIR:
    {
        // CREATE A NEW DIRECTORY
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // TODO: MAKE A PATH BY APPENDING ALL STRINGS/IDENTS



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO CHANGE CURRENT DIR

        BINT err=FSMkdir((char *)path);

        if(err!=FS_OK) {
            rplFSError2Error(err);
        }
        else rplDropData(1);

        return;

    }

    case SDPGDIR:
    {
        // DELETE A DIRECTORY
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // TODO: MAKE A PATH BY APPENDING ALL STRINGS/IDENTS



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO DELETE CURRENT DIR

        BINT err=FSRmdir((char *)path);

        if(err!=FS_OK) {
            rplFSError2Error(err);
        }
        else rplDropData(1);

        return;

    }

    case SDOPENRD:
    {
        // OPEN A FILE FOR READ
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // TODO: MAKE A PATH BY APPENDING ALL STRINGS/IDENTS



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // OPEN THE FILE
        FS_FILE *handle;
        BINT err=FSOpen((char *)path,FSMODE_READ,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

       rplNewBINTPush(FSGetHandle(handle),HEXBINT);

       return;



    }

    case SDCLOSE:
    {
        // CLOSE A HANDLE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));
        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        err=FSClose(handle);
        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

       return;



    }

        // STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors


            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // Library receives:
        // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
        // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

        // VALIDATE RETURNS:
        // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.


        RetNum=OK_CONTINUE;
        return;

    case OPCODE_PROBETOKEN:
        // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
        // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
        {
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);

        return;
        }


    case OPCODE_GETINFO:
        libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

        RetNum=OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {\
        if(MENUNUMBER(MenuCodeArg)>0) { RetNum=ERR_NOTMINE; return; }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)+2];
        RetNum=OK_CONTINUE;
       return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp,(WORDPTR)LIB_HELPTABLE);
       return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError,(WORDPTR)LIB_MSGTABLE);
       return;
    }


    case OPCODE_LIBINSTALL:
        LibraryList=(WORDPTR)libnumberlist;
        RetNum=OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {
        RetNum=ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;


}


#endif