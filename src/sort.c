#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"

bool shouldSwap(Record* rec1,Record* rec2){
    int compareNames = strcmp(rec1->name, rec2->name);
    int compareSurnames = strcmp(rec1->surname, rec2->surname);
    if (compareNames == 0) {
        return compareSurnames > 0; // Swap if surnames need to be adjusted
    }
    return compareNames > 0; // Swap if names need to be adjusted
}

void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
    int totalBlocks;
    BF_GetBlockCounter(file_desc, &totalBlocks);
    int currentBlock = 1;

    while (currentBlock <= totalBlocks) {
        CHUNK chunk;
        chunk.file_desc = file_desc;
        chunk.from_BlockId = currentBlock;
        chunk.to_BlockId = currentBlock + numBlocksInChunk - 1;
        if (chunk.to_BlockId > totalBlocks) {
            chunk.to_BlockId = totalBlocks;
        }
        chunk.blocksInChunk=chunk.to_BlockId-chunk.from_BlockId+1;
        int totalRecords = 0;
        for (int blockId = chunk.from_BlockId; blockId <= chunk.to_BlockId; blockId++) {
            totalRecords += HP_GetRecordCounter(file_desc, blockId);
        }
        chunk.recordsInChunk = totalRecords;
        printf("%d %d %d %d\n",chunk.from_BlockId,chunk.to_BlockId,chunk.recordsInChunk,chunk.blocksInChunk);


        sort_Chunk(&chunk);
        currentBlock += numBlocksInChunk;
    }
}


void sort_Chunk(CHUNK* chunk) {
    int i, j;
    Record record1, record2;

    for (i = 0; i < chunk->recordsInChunk - 1; i++) {
        for (j = 0; j < chunk->recordsInChunk - i - 1; j++) {
            // Get the j-th and (j+1)-th records from the chunk
            CHUNK_GetIthRecordInChunk(chunk, j, &record1);
            CHUNK_GetIthRecordInChunk(chunk, j + 1, &record2);
            // Compare names and surnames of two records and swap if necessary
            if (shouldSwap(&record1, &record2)) {
                CHUNK_UpdateIthRecord(chunk, j, record2);
                CHUNK_UpdateIthRecord(chunk, j + 1, record1);
            }
        }
    }
}


