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
    if(chunk->to_BlockId==iterator->lastBlocksID)
        chunk->recordsInChunk++;

    chunk->blocksInChunk=chunk->to_BlockId-chunk->from_BlockId+1;

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

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record* record) {
    if (iterator == NULL ) {
        return -1; // Return error code if the iterator or record pointer is invalid
    }

    int count=HP_GetRecordCounter(iterator->chunk.file_desc, iterator->currentBlockId);
    // Check if the current block has been fully traversed
    if (iterator->cursor > count) {
        // Move to the next block within the chunk
        iterator->currentBlockId++;
        iterator->cursor = 0;

        // Check if there are more blocks in the chunk
        if (iterator->currentBlockId > iterator->chunk.to_BlockId) {
            record=NULL;
            return -1; // No more records available
        }
    }

    HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId,  iterator->cursor, record);
    HP_Unpin(iterator->chunk.file_desc, iterator->currentBlockId);

    // Move the cursor to the next record
    iterator->cursor++;


    return 0; // Return success code to indicate successful retrieval of the record
}



void CHUNK_DestroyIterator(CHUNK_Iterator *iterator) {
    if (iterator != NULL) {
        free(iterator);
    }
}
