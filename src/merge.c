#include <merge.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc ){
    int totalBlocks;
    BF_GetBlockCounter(input_FileDesc, &totalBlocks);
    int numChunks = totalBlocks / (chunkSize * bWay);
    if (totalBlocks % (chunkSize * bWay) != 0) {
        numChunks++;
    }

    // Perform merging
    for (int i = 0; i < numChunks; ++i) {
        int numBlocksToMerge = chunkSize * bWay;
        if ((i + 1) * chunkSize * bWay > totalBlocks) {
            numBlocksToMerge = totalBlocks - i * chunkSize * bWay;
        }

        // Merge b chunks into a new chunk
        CHUNK newChunk;
        newChunk.file_desc = output_FileDesc;
        newChunk.from_BlockId = (i * chunkSize * bWay) + 1;
        newChunk.to_BlockId = newChunk.from_BlockId + numBlocksToMerge - 1;
        newChunk.blocksInChunk = numBlocksToMerge;
        
        // Create an iterator for merging b chunks
        CHUNK_Iterator iterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);
        CHUNK chunk;
        int chunksMerged = 0;

        // Merge b chunks into a new chunk
        while (chunksMerged < bWay && CHUNK_GetNext(&iterator, &chunk)) {
            for (int blockId = chunk.from_BlockId; blockId <= chunk.to_BlockId; ++blockId) {
                BF_Block* block;
                BF_Block_Init(&block);
                BF_GetBlock(input_FileDesc, blockId, block);

                // Write the block's data to the new chunk
                BF_Block* newBlock;
                BF_Block_Init(&newBlock);
                BF_AllocateBlock(output_FileDesc, newBlock);
                char* data = BF_Block_GetData(block);
                char* newData = BF_Block_GetData(newBlock);
                memcpy(newData, data, BF_BLOCK_SIZE);

                BF_Block_SetDirty(newBlock);
                BF_Block_Destroy(&block);
                BF_Block_Destroy(&newBlock);
            }
            chunksMerged++;
        }

        // Clean up
        CHUNK_Print(newChunk);
        BF_Block* lastBlock;
        BF_Block_Init(&lastBlock);
        BF_GetBlock(output_FileDesc, newChunk.to_BlockId, lastBlock);
        BF_UnpinBlock(lastBlock);
        BF_Block_Destroy(&lastBlock);
        CHUNK_DestroyIterator(&iterator);
    }

    BF_CloseFile(input_FileDesc);
    BF_CloseFile(output_FileDesc);
}
