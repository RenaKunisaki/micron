//Methods dealing with parition tables.
#ifndef _MICRON_LIBS_IO_PARTITION_H_
#define _MICRON_LIBS_IO_PARTITION_H_

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct PACKED {
    uint8_t boot; //0x00=no, 0x80=yes
    uint8_t startC, startH, startS; //cylinder, head, sector
    uint8_t type;
    uint8_t endC, endH, endS;
    uint32_t startSector;
    uint32_t numSectors;
} DosPartitionTableEntry;

typedef struct {
    uint64_t sector; //start sector
    uint64_t size;   //number of sectors
    uint32_t type;   //type ID
} MicronPartition;

int ioGetPartition(FILE *blkdev, int which, MicronPartition *out);

#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_LIBS_IO_PARTITION_H_
