#ifndef _NAND_H_
#define _NAND_H_

#include <stdint.h>

// All NAND code is hardcoded for newtype NANDs with device code 0xda.
// 256Mb 8-bit.

#define NAND_SIZE         0x10000000 // 256Mb
#define NAND_BLOCK_SIZE   0x00020000 // 128kb
#define NAND_PAGE_SIZE    0x00000800 // 2kb
#define NAND_PACKET_SIZE  0x00000200 // 512b

#define NAND_BLOCK_MASK   0xfffe0000
#define NAND_PAGE_MASK    0xfffff800
#define NAND_PACKET_MASK  0xfffffe00

#define NAND_BLOCK_START(addr) (addr & NAND_BLOCK_MASK)
#define NAND_PAGE_START(addr) (addr & NAND_PAGE_MASK)

#define NAND_BLOCK_BITS           17
#define NAND_PAGE_BITS            11
#define NAND_PACKET_BITS           9

#define NAND_NUM_BLOCKS       (NAND_SIZE / NAND_BLOCK_SIZE)       // 2048
#define NAND_PAGES_PER_BLOCK  (NAND_BLOCK_SIZE / NAND_PAGE_SIZE)  // 64
#define NAND_PACKETS_PER_PAGE (NAND_PAGE_SIZE / NAND_PACKET_SIZE) // 4

#define NAND_BLOCK_ADDR(number) (number << NAND_BLOCK_BITS)

#define NAND_BLOCK_NUMBER(addr) (addr >> NAND_BLOCK_BITS)
#define NAND_PAGE_NUMBER(addr) (addr >> NAND_PAGE_BITS)
#define NAND_PACKET_NUMBER_IN_PAGE(addr) ((addr & ~NAND_PAGE_MASK) >> NAND_PACKET_BITS)

#define NAND_STATUS_OK             0
#define NAND_STATUS_TIMEOUT        1
#define NAND_STATUS_BAD            2
#define NAND_STATUS_UNCORRECTABLE  3
#define NAND_STATUS_ILLEGAL_ACCESS 4
#define NAND_STATUS_WRITE_FAIL     5
#define NAND_STATUS_NO_TABLE       6
#define NAND_STATUS_WRITE_PROTECT  7

// Initializes NAND handling and needs to be called before any other NAND function.
// Returns NAND_STATUS.
int NANDInit(void);

// Write protect whole NAND.
void NANDWriteProtect(void);

// Checks if block at @nand_address is bad.
// Returns NAND_STATUS_OK if block @address lies in is ok, NAND_STATUS_BAD if
// it's bad, other NAND_STATUS on error.
int NANDIsBlockBad(uint32_t nand_address);

// Marks block at @nand_address bad.
// Returns NAND_STATUS.
int NANDMarkBlockBad(uint32_t nand_address);

// Block at @nand_address gets erased.
// Returns first error should errors occur.
// Returns NAND_STATUS.
int NANDBlockErase(uint32_t nand_address);

// Reads whole aligned page at @nand_address into @target_address.
// Tries to restore as much information as possible should error occur.
// Returns first error should errors occur.
// Does ECC error correction.
// Does not correct block address.
// Returns NAND_STATUS.
int NANDReadPage(uint32_t nand_address, uint8_t *target_address);

// Reads whole aligned block at @nand_address into @target_address.
// Tries to restore as much information as possible and returns first error
// should errors occur.
// Does ECC error correction.
// Returns NAND_STATUS.
int NANDReadBlock(uint32_t nand_address, uint8_t *target_address);

// Writes data from @source_address to whole aligned page at @nand_address.
// @nand_address needs to be erased before.
// Returns NAND_STATUS.
int NANDWritePage(uint32_t nand_address, uint8_t const *source_address);

// Writes data from @source_address to whole aligned block at @nand_address.
// @nand_address needs to be erased before.
// Bails out on first error.
// Returns NAND_STATUS.
int NANDWriteBlock(uint32_t nand_address, uint8_t const *source_address);

// Reads @num_bytes from @virtual_address to @target_address
// @virtual_address and @num_bytes need not be page aligned.
// Tries to restore as much information as possible and returns first error
// should errors occur.
// Corrects block address according to BL2 block translation table.
// Does ECC error correction.
// Returns NAND_STATUS.
int NANDRead(uint32_t virtual_address, uint8_t *target_address, unsigned int num_bytes);

// Writes @num_bytes from @source_address to @virtual_address
// @virtual_address and @num_bytes need not be page aligned.
// Corrects block address according to BL2 block translation table.
// Bails out on first error to limit damage.
// Returns NAND_STATUS.
int NANDWrite(uint32_t virtual_address, uint8_t const *source_address, unsigned int num_bytes);

#endif
