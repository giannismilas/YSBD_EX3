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

void sort_FileInChunks(int file_desc, int numBlocksInChunk){
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
        sort_Chunk(&chunk);
        currentBlock += numBlocksInChunk;
    }
}

void sort_Chunk(CHUNK* chunk){
    Record* records = malloc(sizeof(Record) * HP_GetMaxRecordsInBlock(chunk->file_desc) * chunk->blocksInChunk);
    int totalRecords = 0;

    // Load all records within the chunk
    for (int blockId = chunk->from_BlockId; blockId <= chunk->to_BlockId; ++blockId) {
        int numRecords = HP_GetRecordCounter(chunk->file_desc, blockId);
        for (int i = 0; i < numRecords; ++i) {
            Record record;
            if (HP_GetRecord(chunk->file_desc, blockId, i, &record) != -1) {
                // Ελέγχουμε αν έχουμε χώρο στον πίνακα records πριν προσθέσουμε την εγγραφή
                if (totalRecords < HP_GetMaxRecordsInBlock(chunk->file_desc) * chunk->blocksInChunk) {
                    records[totalRecords++] = record;
                } else {
                    // Αν ο πίνακας records είναι γεμάτος, μπορούμε να διακόψουμε τη διαδικασία
                    break;
                }
            }
        }
    }

    // Perform sorting
    for (int i = 0; i < totalRecords - 1; ++i) {
        for (int j = 0; j < totalRecords - i - 1; ++j) {
            if (shouldSwap(&records[j], &records[j + 1])) {
                Record temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
            }
        }
    }

    // Rewrite the sorted records back to the chunk
    int currentRecord = 0;
    for (int blockId = chunk->from_BlockId; blockId <= chunk->to_BlockId; ++blockId) {
        int numRecords = HP_GetRecordCounter(chunk->file_desc, blockId);
        for (int i = 0; i < numRecords; ++i) {
            if (currentRecord < totalRecords) {
                if (HP_UpdateRecord(chunk->file_desc, blockId, i, records[currentRecord]) != -1) {
                    currentRecord++;
                }
            } else {
                break;
            }
        }
    }

    free(records);
}