#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName){
    BF_ErrorCode error;
    
    error =  BF_CreateFile(fileName);           					// Create en empty file
    if( error != BF_OK) { BF_PrintError(error); return -1; }
    
    int file;                                   					// File identifier
    error = BF_OpenFile(fileName, &file);
    if( error != BF_OK) { BF_PrintError(error); return -1; }

    BF_Block *block = NULL;
    BF_Block_Init(&block);
    error = BF_AllocateBlock(file,block);      		 				// Allocate the first block of the file
    if( error != BF_OK) { BF_PrintError(error); return -1; }
    
    HP_info* info;
													
    info = (HP_info*) BF_Block_GetData(block);						// Initialization of HP_info 
    info->available_space = BF_BLOCK_SIZE;
    info->last_block = block;                     
    info->last_block_id = 0;                      

    
    BF_Block_SetDirty(block);
    error = BF_UnpinBlock(block);					
    if( error != BF_OK) { BF_PrintError(error); return -1; }

    BF_Block_Destroy(&block);		
    error = BF_CloseFile(file);										// Unpin and destroy before closing the file
    if( error != BF_OK) { BF_PrintError(error); return -1; }
	
    return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){

	BF_ErrorCode error; 
  	error =  BF_OpenFile(fileName, file_desc);								// Open the file that was created earlier
  	if( error != BF_OK) { BF_PrintError(error); exit(EXIT_FAILURE); }
  
  	BF_Block *block = NULL;
  	BF_Block_Init(&block);
  
  	int block_num;									
  	error = BF_GetBlockCounter(*file_desc, &block_num);	 					// Get the first block 
	if( error != BF_OK) { BF_PrintError(error);  exit(EXIT_FAILURE); }		//which holds the wanted information

  	error = BF_GetBlock(*file_desc, block_num - 1 , block);
  	if( error != BF_OK) { BF_PrintError(error);  exit(EXIT_FAILURE); }
		
	HP_info* hp_info;														// Store the information in hp_info
  	hp_info = (HP_info*)BF_Block_GetData(block);

	BF_Block_Destroy(&block);
	
	return hp_info;

 }


int HP_CloseFile(int file_desc,HP_info* hp_info ){
    /*closes the file, unpins and destroyes the file header which stores information about the heap*/
	BF_ErrorCode error;

    BF_Block *block = NULL;
    BF_Block_Init(&block);   
    int block_num;
    error = BF_GetBlockCounter(file_desc, &block_num);								// Get the number of blocks
	if( error != BF_OK) { BF_PrintError(error);  return -1; }


    error = BF_GetBlock(file_desc, block_num - 1 , block);							// Get the first block(file header)
    if( error != BF_OK) { printf("\n"); BF_PrintError(error);  return -1; }
    
    error = BF_UnpinBlock(block);													// Unpin the file header
    if( error != BF_OK) { printf("\n"); BF_PrintError(error);  return -1; }
    BF_Block_Destroy(&block);														// Destroy the file header
    
    
    error = BF_CloseFile(file_desc);												// Close the file
    if( error != BF_OK) { BF_PrintError(error);  return -1; }

	return 0;
}

int HP_InsertEntry(int file_desc, HP_info* hp_info, Record record){
    BF_ErrorCode error;
    int record_size = sizeof(record);

    /*Number of blocks that file_desc has*/
    int blocks_num; 
    error = BF_GetBlockCounter(file_desc, &blocks_num); 
    if( error != BF_OK) { BF_PrintError(error);  return -1; }

    /*Init and allocate blocks*/
    HP_block_info* block_info;
    char* data;

    /*Creating the 2nd block of the file, which is going to be the 1st block to write the record in*/
    if(hp_info->last_block_id == 0){

      	BF_Block *block = NULL;
      	BF_Block_Init(&block);
      	error = BF_AllocateBlock(file_desc,block); 
      	if( error != BF_OK) { BF_PrintError(error);  return -1; }

      	hp_info->last_block = block;                                                    	// last_block is the one that we created
      	hp_info->available_space = BF_BLOCK_SIZE - sizeof(block_info);               		// Available space is BF_BLOCK_SIZE - sizeof(HP_block_info)
      	hp_info->last_block_id++;                                                       	// Last block id = 1 (at the start)

      	BF_Block_SetDirty(hp_info->last_block);
    }
    
    /*Get data from the last_block (the newest) and copy block_info at the start of the block*/
    data = BF_Block_GetData(hp_info->last_block);
    block_info = (HP_block_info*)data;
    
    /*Insert records based to available space*/
    if(record_size <= hp_info->available_space){
		// Copy record after block_info, and after the other records that have been copied
      	memcpy(data + sizeof(block_info) + (block_info->block_records * record_size), &record, record_size);
		
		block_info->block_records++;                                                     	// Records inside current block +1
      	hp_info->available_space = hp_info->available_space - record_size;                	// Update available space
		
	  	hp_info->file_records++;                                                          	// Records inside file(total) +1
      
      	BF_Block_SetDirty(hp_info->last_block);
    }
    else{

    	error = BF_UnpinBlock(hp_info->last_block);                                       	// Unpin the previous block
      	if( error != BF_OK) { BF_PrintError(error); return -1; }
      
		BF_Block *new_block = NULL;                                                       	// Create new block
		BF_Block_Init(&new_block);
		error = BF_AllocateBlock(file_desc, new_block); 
    	if( error != BF_OK) { BF_PrintError(error);  return -1; }
      
		hp_info->last_block = new_block;                                                  	// Now the last block, is the new block created
		hp_info->available_space = BF_BLOCK_SIZE - sizeof(block_info);                 	  	// Available space like in the start	
		hp_info->last_block_id++;
      	block_info->next_block = new_block;
		
      	data = BF_Block_GetData(new_block);                                   				// Get the new data

		block_info = (HP_block_info*)data;
		block_info->block_records = 0;                                                    	// New block, zero records

		//Copy record after block_info because there are no records in the new block yet
		memcpy(data + sizeof(block_info), &record, record_size);
		
		block_info->block_records++;                                                      	// Update block records
		hp_info->file_records++;                                                          	// File records
		hp_info->available_space = hp_info->available_space - record_size;                	// And available space
			
		BF_Block_SetDirty(hp_info->last_block);
    }
    
    return hp_info->last_block_id;

}

int HP_GetAllEntries(int file_desc, HP_info* hp_info, int value){    
    BF_ErrorCode error;
	HP_block_info* block_info;
    Record* record;
	
    BF_Block *block = NULL;                                      					// Create block
    BF_Block_Init(&block);
    error = BF_AllocateBlock(file_desc, block); 
    if( error != BF_OK) { BF_PrintError(error);  return -1; }
    
    int blocks_number;																// Get number of blocks in file
    error = BF_GetBlockCounter(file_desc, &blocks_number);
    if( error != BF_OK) { BF_PrintError(error);  return -1; }

    printf("\n");
	int blocks_read = 0;															// No blocks read yet
    
	for(int i = 0; i < blocks_number; i++){

        error = BF_GetBlock(file_desc, i, block);									// Get block i in file
        if( error != BF_OK) { BF_PrintError(error);  return -1; }
        
		char* data = BF_Block_GetData(block);										// Get it's data
        
		block_info = (HP_block_info*)data;
        record =(Record*)(data + sizeof(block_info));								// First record of block is after block_info
        
		for(int j = 0; j < block_info->block_records; j++) {						// For each record inside the block
        	if(record[j].id == value) {												// If value is found
        		printRecord(record[j]);
          	}	
          
        }
		
		blocks_read++;
		
		error = BF_UnpinBlock(block);
  		if( error != BF_OK) { BF_PrintError(error); return -1; }
    }
	
    return blocks_read;
}
