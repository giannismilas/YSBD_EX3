#include <merge.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>





void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    // Create a CHUNK iterator to traverse through input chunks
    CHUNK_Iterator chunkIterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    CHUNK* ch=malloc(sizeof(CHUNK)* bWay);
    int temp;
    // Allocate memory for storing records and cursors for each chunk
    int* cursors = malloc(sizeof(int) * bWay);

    // Initialize cursors for each chunk
    for (int i = 0; i < bWay; ++i) {
        cursors[i] = -1;
    }

    while (true) {
        bool anyRecords = false;
        // Load new chunks or check records in the loaded chunks
        for (int i = 0; i < bWay; ++i) {
            //printf("%d\t",cursors[i]);
            if (cursors[i] == -1) {  // Load new chunk
                temp=CHUNK_GetNext(&chunkIterator, &ch[i]);
                if (temp!=-1) {
                    if(ch[i].recordsInChunk!=0){
                        cursors[i] = 0;
                        anyRecords = true;
                    }
                }
            } else {  // Check records in the loaded chunks
                anyRecords = true;
            }
             printf("\n");
        }

        if (!anyRecords) {
            break;  // No more records across all chunks
        }

        int minposition = -1;
        Record minRecord;

        // Find the minimum record among the available records in the chunks
        Record current;
        for (int i = 0; i < bWay; ++i) {
            if(cursors[i]!=-1){
                temp=CHUNK_GetIthRecordInChunk(&ch[i],  cursors[i], &current);
                if (temp!=-1 && (minposition == -1 || shouldSwap(&current, &minRecord))) {
                    minRecord = current;
                    minposition = i;
                }
            }
        }
        
        // Write the minRecord to the output file
        HP_InsertEntry(output_FileDesc, minRecord);

        // Move to the next record in the chunk
        cursors[minposition]++;
        if (cursors[minposition] >= ch[minposition].recordsInChunk) {
            cursors[minposition] = -1;  // Chunk exhausted
        }
    }

    // Free allocated memory
    free(ch);
    free(cursors);
}