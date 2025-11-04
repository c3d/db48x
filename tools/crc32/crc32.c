// ****************************************************************************
//  crc32.c                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Compute CRC32 checksum for a file
//
//     This implements the CRC32 algorithm (IEEE 802.3 / Ethernet standard)
//     using the polynomial 0xEDB88320, which matches the output format of
//     the standard Unix crc32 utility.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


// ============================================================================
//
//   CRC32 computation
//
// ============================================================================

static uint32_t crc32_table[256];


static void crc32_init(void)
// ----------------------------------------------------------------------------
//    Initialize CRC32 lookup table
// ----------------------------------------------------------------------------
{
    uint32_t polynomial = 0xEDB88320;

    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ polynomial;
            else
                crc = crc >> 1;
        }
        crc32_table[i] = crc;
    }
}

static uint32_t crc32_compute(const uint8_t *data, size_t length)
// ----------------------------------------------------------------------------
//    Compute CRC32 checksum of data
// ----------------------------------------------------------------------------
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++)
    {
        uint8_t index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[index];
    }

    return ~crc;
}

static uint8_t* read_file(const char *filename, size_t *length)
// ----------------------------------------------------------------------------
//    Read entire file into memory
// ----------------------------------------------------------------------------
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    *length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocate buffer and read
    uint8_t *buffer = malloc(*length);
    if (!buffer)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(fp);
        return NULL;
    }

    size_t read_bytes = fread(buffer, 1, *length, fp);
    fclose(fp);

    if (read_bytes != *length)
    {
        fprintf(stderr, "Error: Failed to read file\n");
        free(buffer);
        return NULL;
    }

    return buffer;
}

int main(int argc, char *argv[])
// ----------------------------------------------------------------------------
//    Main entry point - compute and display CRC32 of a file
// ----------------------------------------------------------------------------
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Initialize CRC32 table
    crc32_init();

    // Read file
    size_t length;
    uint8_t *data = read_file(argv[1], &length);
    if (!data)
        return 1;

    // Compute and print CRC32
    uint32_t crc = crc32_compute(data, length);
    printf("%08x\n", crc);

    free(data);
    return 0;
}

