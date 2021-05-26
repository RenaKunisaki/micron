extern "C" {
    #include <micron.h>
    #include "sdcard.h"
}

int8_t sdFileClsIdx = -1;

int sdFileCls_close(FILE *self) {
    free(self);
    return 0;
}

int sdFileCls_read(FILE *self, void *dest, size_t len) {
    MicronSdCardState *state = (MicronSdCardState*)self->udata.ptr;
    uint32_t block = self->offset / SD_BLOCK_SIZE;
    uint32_t part  = self->offset % SD_BLOCK_SIZE;
    uint8_t *out = (uint8_t*)dest;
    int err = 0, count = 0;
    for(size_t i=0; i<len; i += SD_BLOCK_SIZE) {
        //XXX use sdReadBlocks for large sizes; allow setting timeout?
        size_t remLen = len - i; //remaining length
        if(!part && remLen >= SD_BLOCK_SIZE) {
            //read directly into dest
            err = sdReadBlock(state, block, out, 10000);
        }
        else { //read into buf and copy to dest
            uint8_t buf[SD_BLOCK_SIZE];
            err = sdReadBlock(state, block, buf, 10000);
            if(!err) memcpy(out, &buf[part], MIN((size_t)SD_BLOCK_SIZE, remLen));
        }
        if(err) return err;
        count += SD_BLOCK_SIZE - part;
        self->offset += SD_BLOCK_SIZE - part;
        part = 0;
    }
    return count;
}

int sdFileCls_write(FILE *self, const void *src, size_t len) {
    return -ENOSYS; //TODO
}

int sdFileCls_seek(FILE *self, long int offset, int origin) {
    //XXX can probably move this to a generic method
    MicronSdCardState *state = (MicronSdCardState*)self->udata.ptr;
    switch(origin) {
        case 0: //SEEK_SET
            //printf("SEEK_SET to 0x%llX, size 0x%llX\r\n", offset, state->cardSize);
            if((uint64_t)offset >= state->cardSize) return -ERANGE;
            self->offset = offset;
            return 0;

        case 1: //SEEK_CUR
            //printf("SEEK_CUR by 0x%llX, size 0x%llX\r\n", offset, state->cardSize);
            if((uint64_t)(self->offset + offset) >= state->cardSize) return -ERANGE;
            self->offset += offset;
            return 0;

        case 2: //SEEK_END
            //printf("SEEK_END by 0x%llX, size 0x%llX\r\n", offset, state->cardSize);
            if((uint64_t)offset >= state->cardSize) return -ERANGE;
            self->offset = state->cardSize - offset;
            return 0;

        default: return -EINVAL;
    }
}

int sdFileCls_peek(FILE *self, void *dest, size_t len) {
    return -ENOSYS; //TODO
}

int sdFileCls_getWriteBuf(FILE *self) {
    return -ENOSYS; //TODO
}

int sdFileCls_sync(FILE *self) {
    MicronSdCardState *state = (MicronSdCardState*)self->udata.ptr;
    return spiWaitTxDone(state->port, 10000);
}

int sdFileCls_purge(FILE *self) {
    MicronSdCardState *state = (MicronSdCardState*)self->udata.ptr;
    return spiClear(state->port);
}


MicronFileClass sdFileCls = {
	.close       = sdFileCls_close,
	.read        = sdFileCls_read,
	.write       = sdFileCls_write,
	.seek        = sdFileCls_seek,
	.peek        = sdFileCls_peek,
	.getWriteBuf = sdFileCls_getWriteBuf,
	.sync        = sdFileCls_sync,
	.purge       = sdFileCls_purge,
};

FILE* sdOpenCard(MicronSdCardState *state, int *outErr) {
    int err = 0;
    if(sdFileClsIdx < 0) {
        err = osRegisterFileClass(&sdFileCls);
        if(err < 0) {
            *outErr = err;
            return NULL;
        }
        sdFileClsIdx = err;
    }
    FILE *res = (FILE*)malloc(sizeof(FILE));
    if(!res) {
        *outErr = -ENOMEM;
        return NULL;
    }
    res->fileCls = sdFileClsIdx;
    res->udata.ptr = state;
    res->offset = 0;
    return res;
}
