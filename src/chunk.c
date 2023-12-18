#include <merge.h>
#include <stdio.h>
#include "chunk.h"


CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
    CHUNK_Iterator iterator;
    iterator.file_desc = fileDesc;
    iterator.current = 1;  // Starting block
    iterator.blocksInChunk = blocksInChunk;
    BF_GetBlockCounter(fileDesc, &iterator.lastBlocksID);

    return iterator;
}


int CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK *chunk) {
    if (iterator->current > iterator->lastBlocksID) {
        return -1; // Reached the end of chunks
    }
    
    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = iterator->current;
    chunk->to_BlockId = iterator->current + iterator->blocksInChunk - 1;

    // Adjust the 'to_BlockId' if it exceeds the limit
    if (chunk->to_BlockId > iterator->lastBlocksID) {
        chunk->to_BlockId = iterator->lastBlocksID;
    }

    int count=0;
    for(int i=chunk->from_BlockId; i<=chunk->to_BlockId; i++)
        count+=HP_GetRecordCounter(iterator->file_desc,i);
    chunk->recordsInChunk=count;

    iterator->current += iterator->blocksInChunk;

    return 0; // Successfully retrieved the next chunk
}


int CHUNK_GetIthRecordInChunk(CHUNK* chunk,  int i, Record* record){
    int blockId = chunk->from_BlockId + (i - 1) / HP_GetMaxRecordsInBlock(chunk->file_desc);
    int cursor = (i - 1) % HP_GetMaxRecordsInBlock(chunk->file_desc);
    int temp= HP_GetRecord(chunk->file_desc, blockId, cursor, record);
    HP_Unpin(chunk->file_desc, blockId);
    return temp;
}

int CHUNK_UpdateIthRecord(CHUNK* chunk,  int i, Record record){
    int blockId = chunk->from_BlockId + (i - 1) / HP_GetMaxRecordsInBlock(chunk->file_desc);
    int cursor = (i - 1) % HP_GetMaxRecordsInBlock(chunk->file_desc);
    int temp= HP_UpdateRecord(chunk->file_desc, blockId, cursor, record);
    HP_Unpin(chunk->file_desc, blockId);
    return temp;
}

void CHUNK_Print(CHUNK chunk){
    int blockId;
    for (blockId = chunk.from_BlockId; blockId <= chunk.to_BlockId; ++blockId) {
        HP_PrintBlockEntries(chunk.file_desc, blockId);
    }
}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk){
    CHUNK_RecordIterator iterator;
    iterator.chunk = *chunk;
    iterator.currentBlockId = chunk->from_BlockId;
    iterator.cursor = 0;
    return iterator;
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator,Record* record){
    int maxRecords = HP_GetMaxRecordsInBlock(iterator->chunk.file_desc);
    if (iterator->currentBlockId > iterator->chunk.to_BlockId) {
        return -1; // End of chunk
    }
    int result = HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId, iterator->cursor, record);
    if (result == -1) {
        // Move to the next block if current block is exhausted
        iterator->currentBlockId++;
        iterator->cursor = 0;
        return CHUNK_GetNextRecord(iterator, record);
    } else {
        iterator->cursor++;
        if (iterator->cursor == maxRecords) {
            // Move to the next block if cursor reaches block's record limit
            iterator->currentBlockId++;
            iterator->cursor = 0;
        }
        return 0;
    }
}


void CHUNK_DestroyIterator(CHUNK_Iterator *iterator) {
    if (iterator != NULL) {
        free(iterator);
    }
}
