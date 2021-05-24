#ifdef __cplusplus
	extern "C" {
#endif
#include <micron.h>

int ioGetPartition(FILE *blkdev, int which, MicronPartition *out) {
    /** Get partition information.
     *  @param blkdev File to read from.
     *  @param which Partition index.
     *  @param out Receives information about specified partition.
     *  @return zero, or negative error code.
     *  @note Only DOS partition tables are currently supported.
     */
    int err;
    uint8_t data[512];
    if(which > 3) {
        #if FAT_DEBUG_PRINT
            printf("FAT: Extended partitions not supported yet\r\n");
        #endif
        return -ENOSYS; //XXX extended partitions not supported
    }

    err = fseek(blkdev, 0, SEEK_SET);
    if(err) return err;
    err = read(blkdev, data, 512);
    if(err < 0) return err;

    if(data[0x1FE] != 0x55 || data[0x1FF] != 0xAA) {
        #if FAT_DEBUG_PRINT
            printf("FAT: Bad MBR signature 0x%02X%02X, expected 0x55AA\r\n",
                data[0x1FE], data[0x1FF]);
        #endif
        //bad MBR signature
        return -EILSEQ;
    }

    //XXX support other partition table types, CHS sizing
    uint32_t offs = 0x1BE + (which * 16);
    DosPartitionTableEntry *ent = (DosPartitionTableEntry*)&data[offs];
    #if 0
    printf("Partition %d: boot=%08X start=%08X,%08X,%08X end=%08X,%08X,%08X type=%02X startSector=%08X numSectors=%08X\r\n",
        which, ent->boot, ent->startC, ent->startH, ent->startS,
        ent->endC, ent->endH, ent->endS, ent->type, ent->startSector,
        ent->numSectors);
    #endif
    out->sector = ent->startSector;
    out->size   = ent->numSectors;
    out->type   = ent->type;

    return 0;
}


#ifdef __cplusplus
	} //extern "C"
#endif
