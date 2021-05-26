extern "C" {
    #include <micron.h>
    #include "fat.h"
}

void fatDecodeDate(uint16_t date, uint16_t *year, uint8_t *month, uint8_t *day){
    *day   =  date       & 0x1F;
    *month = (date >> 5) & 0x0F;
    *year  = (date >> 9) + 1980;
}

void fatDecodeTime(uint16_t time, uint8_t *hour, uint8_t *minute,
uint8_t *second) {
    *second = (time        & 0x1F) * 2;
    *minute = (time >>  5) & 0x3F;
    *hour   =  time >> 11;
}

int _readSector(FILE *blkdev, uint64_t sector, void *out) {
    int err = fseek(blkdev, sector * FAT_SECTOR_SIZE, SEEK_SET);
    if(err < 0) {
        #if FAT_DEBUG_PRINT
            printf("FAT: seek to sector 0x%llX failed: %d\r\n", sector, err);
        #endif
        return err;
    }
    return read(blkdev, out, FAT_SECTOR_SIZE);
}

int fatGetMBR(FILE *blkdev, uint64_t sector, fat32_mbr *out, uint32_t timeout) {
    int err = _readSector(blkdev, sector, out);
    if(err < 0) return err;

    if(out->mbrSig != 0xAA55) {
        #if FAT_DEBUG_PRINT
        printf("FAT: Bad MBR signature 0x%04X, expected 0xAA55\r\n",
            out->mbrSig);
        #endif
        return -EILSEQ;
    }
    out->_micron_startSector = sector;

    #if FAT_DEBUG_PRINT
        char oemName[16], volName[16], fatName[16];
        memset(oemName, 0, sizeof(oemName));
        memset(volName, 0, sizeof(volName));
        memset(fatName, 0, sizeof(fatName));
        strncpy(oemName, out->oemName,  8);
        strncpy(volName, out->volName, 11);
        strncpy(fatName, out->fatName,  8);

        printf("FAT MBR at sector 0x%08llx:\r\n", sector);
        printf("  OEM Name:          '%s'\r\n",   oemName);
        printf("  bytesPerSector:    0x%04X\r\n", out->bytesPerSector);
        printf("  sectorsPerCluster: 0x%02X\r\n", out->sectorsPerCluster);
        printf("  reservedSectors:   0x%04X\r\n", out->reservedSectors);
        printf("  numFats:           0x%02X\r\n", out->numFats);
        printf("  maxRootDirEnts:    0x%04X\r\n", out->maxRootDirEnts);
        printf("  numSmallSectors:   0x%04X\r\n", out->numSmallSectors);
        printf("  mediaDescriptor:   0x%02X\r\n", out->mediaDescriptor);
        printf("  sectorsPerFat:     0x%04X\r\n", out->sectorsPerFat);
        printf("  sectorsPerTrack:   0x%04X\r\n", out->sectorsPerTrack);
        printf("  numHeads:          0x%04X\r\n", out->numHeads);
        printf("  numHiddenSectors:  0x%08lX\r\n", out->numHiddenSectors);
        printf("  numSectors:        0x%08lX\r\n", out->numSectors);
        printf("  sectorsPerFat32:   0x%08lX\r\n", out->sectorsPerFat32);
        printf("  flags:             0x%04X\r\n", out->flags);
        printf("  version:           0x%04X\r\n", out->version);
        printf("  rootCluster:       0x%08lX\r\n", out->rootCluster);
        printf("  fsInfoSector:      0x%04X\r\n", out->fsInfoSector);
        printf("  mbrBackupSector:   0x%04X\r\n", out->mbrBackupSector);
        printf("  driveNum:          0x%04X\r\n", out->driveNum);
        printf("  extSig:            0x%02X\r\n", out->extSig);
        printf("  serial:            0x%08lX\r\n", out->serial);
        printf("  volName:           '%s'\r\n",   volName);
        printf("  fatName:           '%s'\r\n",   fatName);
        printf("  mbrSig:            0x%04X\r\n", out->mbrSig);
    #endif

    return 0;
}


int fatGetFsInfo(FILE *blkdev, fat32_mbr *mbr, fat32_fsinfo *out,
uint32_t timeout) {
    int err = _readSector(blkdev,
        mbr->fsInfoSector + mbr->_micron_startSector, out);
    if(err < 0) {
        #if FAT_DEBUG_PRINT
            printf("FAT: Read MBR sector failed: %d\r\n", err);
        #endif
        return err;
    }

    if(out->mbrSig != 0xAA55) {
        #if FAT_DEBUG_PRINT
        printf("FAT: Bad Fsinfo MBR signature 0x%04X, expected 0xAA55\r\n",
            out->mbrSig);
        #endif
        return -EILSEQ;
    }

    #if FAT_DEBUG_PRINT
        printf("FAT FS info:\r\n");
        printf("  signature:       0x%08lX\r\n", out->signature);
        printf("  fsInfoSig:       0x%08lX\r\n", out->fsInfoSig);
        printf("  numFreeClusters: 0x%08lX\r\n", out->numFreeClusters);
        printf("  lastUsedCluster: 0x%08lX\r\n", out->lastUsedCluster);
        printf("  mbrSig:          0x%04X\r\n", out->mbrSig);
    #endif

    return 0;
}


int fatGetNextCluster(FILE *blkdev, fat32_mbr *mbr, int cluster, uint32_t timeout) {
    uint64_t mapSector = mbr->_micron_startSector + mbr->reservedSectors +
        ((cluster * 4) / FAT_SECTOR_SIZE);
    uint32_t map[FAT_SECTOR_SIZE / 4];

    #if FAT_DEBUG_PRINT
        printf("Read cluster map item %d from 0x%08llX: start=0x%08llX rsvd=0x%08X\r\n",
            cluster, mapSector, mbr->_micron_startSector, mbr->reservedSectors);
    #endif
    int err = _readSector(blkdev, mapSector, map);
    if(err < 0) {
        #if FAT_DEBUG_PRINT
            printf("FAT: Read cluster map sector failed: %d\r\n", err);
        #endif
        return err;
    }


    #if FAT_DEBUG_PRINT
        printf("FAT: Cluster map at sector %d:\r\n", cluster);
        for(int i=0; i<FAT_SECTOR_SIZE/4; i += 4) {
            for(int j=0; j<4; j++) {
                printf("%08X ", map[i+j]);
            }
            printf("\r\n");
        }
    #endif

    int idx = cluster % (FAT_SECTOR_SIZE / 4);
    int r = map[idx] & 0x0FFFFFFF;
    #if FAT_DEBUG_PRINT
        printf("cluster[%d]: %08lX -> %08X\r\n", idx, map[idx], r);
    #endif
    if(r < 2 || r >= 0x0FFFFFF0) return 0;
    return r;

    //each entry in the cluster map is:
    //- a cluster number, meaning the file occupies THIS cluster;
    //  the number is which map entry is next
    //- 0: free cluster
    //- 1: reserved
    //- 0x0FFFFFFx where x in 0-6: reserved
    //- 0x0FFFFFF7: bad cluster
    //- 0x0FFFFFFx where x in 8-F: this is the last cluster of this file
    //each entry must be masked with 0x0FFFFFFF.
}


int fatGetDirEntry(FILE *blkdev, fat32_mbr *mbr, uint32_t idx,
fat32_dirent *out, uint32_t timeout) {
    uint64_t dataSector = mbr->_micron_startSector + mbr->reservedSectors +
        (mbr->numFats * mbr->sectorsPerFat32) +
        ((idx * sizeof(fat32_dirent)) / FAT_SECTOR_SIZE);

    //read the entry
    uint8_t buffer[FAT_SECTOR_SIZE];
    int err = _readSector(blkdev, dataSector, buffer);
    if(err < 0) return err;

    uint32_t bPos = (idx * sizeof(fat32_dirent)) % FAT_SECTOR_SIZE;
    memcpy(out, &buffer[bPos], sizeof(fat32_dirent));

    #if 0 && FAT_DEBUG_PRINT
        char shortName[16], shortExt[4];
        memset(shortName, 0, sizeof(shortName));
        memset(shortExt,  0, sizeof(shortExt));
        strncpy(shortName, out->shortName,  8);
        strncpy(shortExt,  out->shortExt,   3);

        uint16_t year; uint8_t month, day;
        uint8_t hour, minute, second;

        printf("FAT dir entry 0x%08lX:\r\n", idx);
        printf("  Short name: '%-8s.%-3s'\r\n", shortName, shortExt);
        printf("  Attributes: 0x%02X %c%c%c%c%c%c%c%c\r\n", out->attributes,
            (out->attributes & FAT_ATTR_READONLY)     ? 'R' : '-',
            (out->attributes & FAT_ATTR_HIDDEN)       ? 'H' : '-',
            (out->attributes & FAT_ATTR_SYSTEM)       ? 'S' : '-',
            (out->attributes & FAT_ATTR_VOLUME_LABEL) ? 'L' : '-',
            (out->attributes & FAT_ATTR_DIRECTORY)    ? 'D' : '-',
            (out->attributes & FAT_ATTR_ARCHIVE)      ? 'A' : '-',
            (out->attributes & FAT_ATTR_DEVICE)       ? 'V' : '-',
            (out->attributes & 0x80)                  ? 'X' : '-');
        printf("  ExtAttrs:   0x%02X\r\n", out->extAttributes);

        fatDecodeDate(out->createDate, &year, &month, &day);
        fatDecodeTime(out->createTime, &hour, &minute, &second);
        if(out->createCsec > 99) second++;
        printf("  Created:    %04d-%02d-%02d %02d:%02d:%02d.%02d\r\n",
            year, month, day, hour, minute, second, out->createCsec % 100);

        fatDecodeDate(out->accessDate, &year, &month, &day);
        printf("  Accessed:   %04d-%02d-%02d\r\n", year, month, day);

        fatDecodeDate(out->modifyDate, &year, &month, &day);
        fatDecodeTime(out->modifyTime, &hour, &minute, &second);
        printf("  Modified:   %04d-%02d-%02d %02d:%02d:%02d\r\n",
            year, month, day, hour, minute, second);

        printf("  Start cluster: 0x%04X%04X\r\n",
            out->startClusterHi, out->startClusterLo);
        printf("  Size:          0x%08lX\r\n", out->size);
    #endif

    return 0;
}


int fatReadDir(FILE *blkdev, fat32_mbr *mbr, int idx, micronDirent *out,
uint32_t timeout) {
    int err;
    fat32_dirent dir;
    //uint16_t year;
    //uint8_t month, day, hour, minute, second;
    char longName[2048];
    longName[0] = '\0';

    //really, a directory is just a file with the DIRECTORY attribute,
    //whose contents are `fat32_dirent`s.
    while(1) {
        err = fatGetDirEntry(blkdev, mbr, idx++, &dir, timeout);
        if(err) return err;
        if(dir.shortName[0] == 0) return -ENOENT;
        if(dir.shortName[0] == 0xE5) {
            //deleted entry. XXX what do?
            continue;
        }
        if(dir.attributes == 0x0F) {
            //This is a long file name entry
            vfat_lfn *lfn = (vfat_lfn*)&dir;
            int seq = (lfn->seq & 0x1F) - 1;
            char *name = &longName[seq*13];
            //XXX UTF-8/UCS-2?
            for(int i=0; i<5; i++) name[i   ] = lfn->name0[i] & 0xFF;
            for(int i=0; i<6; i++) name[i+5 ] = lfn->name1[i] & 0xFF;
            for(int i=0; i<2; i++) name[i+11] = lfn->name2[i] & 0xFF;
            continue;
        }
        break;
    }

    //XXX decode date/time fields
    out->attributes = dir.attributes | (dir.extAttributes << 8);
    out->cluster    = dir.startClusterLo | (dir.startClusterHi << 16);
    out->size       = dir.size;

    if(longName[0] == '\0') {
        char shortName[16], shortExt[4];
        memset(shortName, 0, sizeof(shortName));
        memset(shortExt,  0, sizeof(shortExt));
        strncpy(shortName, dir.shortName,  8);
        strncpy(shortExt,  dir.shortExt,   3);
        snprintf(out->name, sizeof(out->name), "%-8s.%-3s",
            shortName, shortExt);
    }
    else strncpy(out->name, longName, sizeof(out->name));
    return idx;
}


int fatReadFile(FILE *blkdev, fat32_mbr *mbr, micronDirent *file,
uint32_t offset, uint32_t size, void *out, uint32_t timeout) {
    int err;
    uint8_t  *dest = (uint8_t*)out;
    uint64_t start = mbr->_micron_startSector;
    uint32_t clusterSize = mbr->sectorsPerCluster * mbr->bytesPerSector;
    uint32_t clusterIdx  = offset / clusterSize;
    uint64_t dataSector  = start + (mbr->numFats * mbr->sectorsPerFat32);

    //Read the cluster map for the first cluster.
    //XXX cache this stuff
    uint32_t map[FAT_SECTOR_SIZE / 4];
    uint64_t prevSector = 0;
    int cluster = file->cluster;
    for(uint32_t i=0; i<clusterIdx; i++) {
        uint64_t mapSector = start + mbr->reservedSectors +
            ((cluster * 4) / FAT_SECTOR_SIZE);

        if(mapSector != prevSector) {
            err = _readSector(blkdev, mapSector, map);
            if(err < 0) return err;
        }
        prevSector = mapSector;

        int idx = cluster % (FAT_SECTOR_SIZE / 4);
        int r = map[idx] & 0x0FFFFFFF;
        if(r < 2 || r >= 0x0FFFFFF0) return -ERANGE;
        cluster = r;
    }

    //read data
    uint32_t destOffs = 0;
    while(size > 0) {
        //read that cluster
        uint8_t buffer[FAT_SECTOR_SIZE];
        uint64_t sector = ((cluster * clusterSize) / FAT_SECTOR_SIZE) + dataSector;
        //printf("Read cluster %d => sector 0x%08llX\r\n", cluster, sector);
        err = _readSector(blkdev, sector, buffer);
        if(err < 0) return err;
        memcpy((void*)&dest[destOffs], buffer, MIN(size, (size_t)FAT_SECTOR_SIZE));
        destOffs += MIN(size, (size_t)FAT_SECTOR_SIZE);

        if(size > FAT_SECTOR_SIZE) size -= FAT_SECTOR_SIZE;
        else break;

        cluster = fatGetNextCluster(blkdev, mbr, cluster, timeout);
        if(cluster <  0) return cluster;
        if(cluster == 0) break;
    }

    return destOffs;
}


int fatGetInfo(FILE *blkdev, uint64_t sector, uint32_t timeout) {
    int err;
    fat32_mbr mbr;

    //XXX what if mbr.bytesPerSector != device sector size?
    //apparently that's not well supported in general.

    err = fatGetMBR(blkdev, sector, &mbr, timeout);
    if(err) return err;

    //read fsinfo (not actually needed...)
    fat32_fsinfo fsinfo;
    err = fatGetFsInfo(blkdev, &mbr, &fsinfo, timeout);
    if(err) return err;

    //read root dir
    int idx = 0;

    //printf("FileName.Ext Attribs  Ex  FileSize Created                Accessed   Modified            1stCluster LongName\r\n");
    printf("Attribs      FileSize 1stCluster NextClustr Name\r\n");
    while(1) {
        micronDirent dir;
        err = fatReadDir(blkdev, &mbr, idx, &dir, timeout);
        if(err == -ENOENT) break;
        else if(err < 0) return err;
        else idx = err; //returns next index

        printf("%c%c%c%c%c%c%c%c %12llu 0x%08llX 0x%08X ",
            (dir.attributes & FAT_ATTR_READONLY)     ? 'R' : '-',
            (dir.attributes & FAT_ATTR_HIDDEN)       ? 'H' : '-',
            (dir.attributes & FAT_ATTR_SYSTEM)       ? 'S' : '-',
            (dir.attributes & FAT_ATTR_VOLUME_LABEL) ? 'L' : '-',
            (dir.attributes & FAT_ATTR_DIRECTORY)    ? 'D' : '-',
            (dir.attributes & FAT_ATTR_ARCHIVE)      ? 'A' : '-',
            (dir.attributes & FAT_ATTR_DEVICE)       ? 'V' : '-',
            (dir.attributes & 0x80)                  ? 'X' : '-',
            dir.size, dir.cluster, idx);
        for(int i=0; dir.name[i]; i++) {
            char c = dir.name[i];
            if(c >= 0x20 && c <= 0x7E) putc(c, stdout);
            else printf("\\x%02X", c);
        }
        printf("\r\n");

        /*
        fatDecodeDate(dir.createDate, &year, &month, &day);
        fatDecodeTime(dir.createTime, &hour, &minute, &second);
        if(dir.createCsec > 99) second++;
        printf("%04d-%02d-%02d %02d:%02d:%02d.%02d ",
            year, month, day, hour, minute, second, dir.createCsec % 100);

        fatDecodeDate(dir.accessDate, &year, &month, &day);
        printf("%04d-%02d-%02d ", year, month, day);

        fatDecodeDate(dir.modifyDate, &year, &month, &day);
        fatDecodeTime(dir.modifyTime, &hour, &minute, &second);
        printf("%04d-%02d-%02d %02d:%02d:%02d ",
            year, month, day, hour, minute, second);

        printf("0x%04X%04X '%s'\r\n",
            dir.startClusterHi, dir.startClusterLo, longName);
        */

        /* uint8_t buffer[512];
        int r = fatReadFile(blkdev, &mbr, &dir, 0, 512, buffer, timeout);
        printf("read: %d: ", r);
        for(int i=0; i<16; i++) {
            char c = buffer[i];
            if(c < 0x20 || c > 0x7E) printf("\\x%02X", c);
            else printf("%c", c);
        }
        printf("\r\n"); */
    }

    return 0;
}
