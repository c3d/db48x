/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

// SCAN A CLUSTER CHAIN AND CONVERT TO FRAGMENTS

int FSGetChain(int firstcluster, FS_FRAGMENT * fr, FS_VOLUME * fs)
{
    int addr, fatentry = 0, anotherblock, eoc, bufaddr;
    FS_FRAGMENT *orgfr, *temp;
    unsigned char *buffer, lostbyte = 0;

    if(firstcluster == 0) {
        fr->StartAddr = fr->EndAddr = 0;
        fr->NextFragment = NULL;
        return FS_OK;
    }

    if((firstcluster < 2)
            || ((unsigned int)firstcluster > fs->TotalClusters + 1)) {
        fr->StartAddr = fr->EndAddr = 0;
        fr->NextFragment = NULL;
        return FS_ERROR;
    }

    if(fr->StartAddr != 0)
        return FS_OK;   // CHAIN ALREADY EXISTS, DO NOT REFRESH

    buffer = simpmallocb(512);
    if(!buffer)
        return FS_ERROR;

// SET 32-BYTE BLOCKS
//if(!SDDSetBlockLen(fs->Disk,5)) return FS_ERROR;
//printf("Blk OK\n");
    orgfr = fr;

    eoc = 0;

    do {
        anotherblock = 1;
        fr->NextFragment = NULL;
//printf("Start fr=%d\n",firstcluster);
        fr->StartAddr = FSCluster2Addr(firstcluster, fs);
        addr = FSCluster2FATEntry(firstcluster, fs);
//printf("addr=%08X\n",addr);
        bufaddr = addr & (~511);
        addr &= 511;
        do {

//printf("read=%08X\n",bufaddr);
            if(SDDRead((((uint64_t) fs->FirstFATAddr) << 9) + bufaddr, 512,
                        buffer, fs->Disk) != 512) {
// FREE ENTIRE CHAIN
                simpfree(buffer);
                fr = orgfr->NextFragment;
                while(fr != NULL) {
                    temp = fr;
                    fr = fr->NextFragment;
                    simpfree(temp);
                }
                orgfr->NextFragment = NULL;
                return FS_ERROR;
            }
//printf("Read OK\n");
            FSPatchFATBlock(buffer, 512, bufaddr, fs, FALSE);

            switch (fs->FATType) {
            case 1:
                for(; addr <= 510; ++addr, ++firstcluster) {
                    if(addr == -1)
                        fatentry = lostbyte + (buffer[0] << 8);
                    else
                        fatentry = ReadInt16(buffer + addr);
                    if(firstcluster & 1)
                        fatentry >>= 4;
                    else
                        fatentry &= 0xfff;
                    if(fatentry != firstcluster + 1) {
                        anotherblock = 0;
                        break;
                    }
                    if(firstcluster & 1)
                        ++addr;
                }
                if(addr == 511)
                    lostbyte = buffer[511];     // CARRYOVER FOR NEXT BLOCK
                if(fatentry > 0xff7)
                    eoc = 1;
                break;

            case 2:
                for(; addr <= 510; addr += 2, ++firstcluster) {
                    fatentry = ReadInt16(buffer + addr);
                    //printf("Entry=%04X fc=%04X\n",fatentry,firstcluster);
                    //keyb_getkeyM(1);
                    if(fatentry != firstcluster + 1) {
//              printf("End of fragment\nnewcluster=%d\ncurrent=%d\n",fatentry,firstcluster);
                        anotherblock = 0;
                        break;
                    }
                }
                if(fatentry > 0xfff7)
                    eoc = 1;
                break;

            case 3:
                for(; addr <= 508; addr += 4, ++firstcluster) {
                    fatentry =
                            (*((unsigned int *)(buffer + addr))) & 0x0fffffff;
                    if(fatentry != firstcluster + 1) {
                        anotherblock = 0;
                        break;
                    }
                }
                if(fatentry > 0x0ffffff7)
                    eoc = 1;
                break;
            }
//printf("end of for loops\n");
            addr -= 512;
            bufaddr += 512;
        }
        while(anotherblock && !eoc);

// END OF A FRAGMENT WAS FOUND
        fr->EndAddr =
                FSCluster2Addr(firstcluster, fs) + (1 << (fs->ClusterSize - 9));
//printf("end of frag\n");
//keyb_getkeyM(1);
        if(!eoc) {
            if(!(fr->NextFragment =
                        (FS_FRAGMENT *) simpmallocb(sizeof(FS_FRAGMENT)))) {
// FREE ENTIRE CHAIN
                fr = orgfr->NextFragment;
                while(fr != NULL) {
                    temp = fr;
                    fr = fr->NextFragment;
                    simpfree(temp);
                }
                orgfr->NextFragment = NULL;
                return FS_ERROR;
            }

            fr = fr->NextFragment;
            firstcluster = fatentry;
        }

    }
    while(!eoc);

    simpfree(buffer);
    return FS_OK;
}

#endif
