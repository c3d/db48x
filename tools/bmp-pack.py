#!/usr/bin/env python3
# ****************************************************************************
#  bmp-pack.py                                                  DB48X project
# ****************************************************************************
#
#   File Description:
#
#     Compress the 1-bit BMP help images for the calculator disk
#
#     Each row is combined by XOR with the row above it, so that identical
#     rows become zeros, then the whole image is coded with PackBits.
#     The file keeps its BMP header, with a private compression code, and
#     imageSize still gives the uncompressed size, from which the reader
#     computes the row stride. files::recall_grob decodes it on the fly.
#
#     Usage: bmp-pack.py file.bmp...   (rewrites files in place)
#     Files that are not 1-bit uncompressed BMP, or that would not get
#     smaller, are left untouched.
#
# ****************************************************************************

import struct
import sys

XOR_PACKBITS = 0x31504258           # 'XBP1', see files.cc


def packbits(data):
    out = bytearray()
    i, n = 0, len(data)
    while i < n:
        j = i
        while j + 1 < n and data[j + 1] == data[i] and j - i < 127:
            j += 1
        if j > i:                                   # Run of j-i+1 equal bytes
            out += bytes([257 - (j - i + 1), data[i]])
            i = j + 1
        else:                                       # Literal bytes
            k = i
            while k < n and k - i < 128 and not (k + 1 < n and data[k + 1] == data[k]):
                k += 1
            k = max(k, i + 1)
            out += bytes([k - i - 1]) + data[i:k]
            i = k
    return bytes(out)


def unpackbits(data, size):
    out = bytearray()
    i = 0
    while len(out) < size:
        h = data[i]
        i += 1
        if h < 128:
            out += data[i:i + h + 1]
            i += h + 1
        else:
            out += bytes([data[i]]) * (257 - h)
            i += 1
    return bytes(out[:size])


def pack(name):
    d = open(name, 'rb').read()
    if d[:2] != b'BM':
        return None
    off, = struct.unpack_from('<I', d, 10)
    hsize, w, h, planes, bpp, comp, isize = struct.unpack_from('<IiiHHII', d, 14)
    if hsize != 40 or bpp != 1 or comp != 0 or off != 62 or h <= 0:
        return None
    stride = isize // h
    pixels = d[off:off + stride * h]
    rows = [pixels[r * stride:(r + 1) * stride] for r in range(h)]
    # BMP rows are bottom-up; the reader decodes them in file order
    xored, prev = bytearray(), bytes(stride)
    for row in rows:
        xored += bytes(a ^ b for a, b in zip(row, prev))
        prev = row
    packed = packbits(bytes(xored))
    if len(packed) >= len(pixels):
        return None
    # Check the round trip before writing anything
    back, prev = unpackbits(packed, len(xored)), bytes(stride)
    check = bytearray()
    for r in range(h):
        row = bytes(a ^ b for a, b in zip(back[r * stride:(r + 1) * stride], prev))
        check += row
        prev = row
    assert bytes(check) == pixels, name
    hdr = bytearray(d[:off])
    struct.pack_into('<I', hdr, 2, off + len(packed))           # File size
    struct.pack_into('<I', hdr, 30, XOR_PACKBITS)               # Compression
    open(name, 'wb').write(bytes(hdr) + packed)
    return len(d), off + len(packed)


before = after = count = 0
for name in sys.argv[1:]:
    r = pack(name)
    if r:
        count += 1
        before += r[0]
        after += r[1]
if count:
    print(f'bmp-pack: {count} images, {before // 1024} KB to {after // 1024} KB')
