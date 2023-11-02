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
    
    /*ΔΕΝ ΘΕΛΕΙ CREATE ΓΙΑΤΙ ΥΠΑΡΧΕΙ ΗΔΗ ΤΟ ΑΡΧΕΙΟ, ΑΠΛΑ ΚΑΝΟΥΜΕ OPENFILE*/
    /*ΑΝ ΚΑΝΕΙΣ CREATE ΘΑ ΒΓΑΛΕΙ ERRORCODE 4, ΔΗΛΑΔΗ BF_FILE_ALREADY_EXISTS*/
    error =  BF_CreateFile(fileName);           // create en empty file
    if( error != BF_OK) { BF_PrintError(error); return -1; }
    printf("create:filename:%d\n", *fileName);
    
    int file;                                   // file reco
    error = BF_OpenFile(fileName, &file);
    if( error != BF_OK) { BF_PrintError(error); return -1; }
    printf("create_file:filedesc:%d\n",file);

    BF_Block *block = NULL;
    BF_Block_Init(&block);
    error = BF_AllocateBlock(file,block);       //allocate the first block of the file
    if( error != BF_OK) { BF_PrintError(error); return -1; }
    
    HP_info* info;
    
    info = (HP_info*) BF_Block_GetData(block);
    info->available_space = BF_BLOCK_SIZE;
    info->last_block = block;                     //GIA KAPOIO LOGO MOLIS VGAINEI APO TH SINARTISI
    info->last_block_id = 0;                      //MIDENIZONTAI KAI GINONTAI NULL TA PEDIA AUTA

    
    BF_Block_SetDirty(block);
    error = BF_UnpinBlock(block);
    if( error != BF_OK) { BF_PrintError(error); return -1; }

    BF_Block_Destroy(&block);
    error = BF_CloseFile(file);
    if( error != BF_OK) { BF_PrintError(error); return -1; }
	
    return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){    
  printf("open:filename:%d\n", *fileName);

	BF_ErrorCode error; 
  error =  BF_OpenFile(fileName, file_desc);
  if( error != BF_OK) { BF_PrintError(error); exit(EXIT_FAILURE); }
   
  printf("open:filedesc:%d\n", *file_desc);
  
  BF_Block *block = NULL;
  BF_Block_Init(&block);
  
  
  int block_num;
  error = BF_GetBlockCounter(*file_desc, &block_num);
	if( error != BF_OK) { BF_PrintError(error);  exit(EXIT_FAILURE); }

  //printf("number_of_blocks:%d\n", block_num);

  error = BF_GetBlock(*file_desc, block_num - 1 , block);
  if( error != BF_OK) { BF_PrintError(error);  exit(EXIT_FAILURE); }
		
	HP_info* hp_info;
  hp_info = (HP_info*)BF_Block_GetData(block);

	BF_Block_Destroy(&block);
	
	return hp_info;

 }


int HP_CloseFile(int file_desc,HP_info* hp_info ){
    BF_ErrorCode error;

    BF_Block *block = NULL;
    BF_Block_Init(&block);   
    int block_num;
    error = BF_GetBlockCounter(file_desc, &block_num);
	  if( error != BF_OK) { BF_PrintError(error);  return -1; }


    error = BF_GetBlock(file_desc, block_num - 1 , block);
    if( error != BF_OK) { printf("\n"); BF_PrintError(error);  return -1; }
    
    error = BF_UnpinBlock(block);
    if( error != BF_OK) { printf("\n"); BF_PrintError(error);  return -1; }
    BF_Block_Destroy(&block);
    
    
    error = BF_CloseFile(file_desc);
    if( error != BF_OK) { BF_PrintError(error);  return -1; }

    printf("\ngooood\n");
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
      	hp_info->last_block_id++;                                                       	// Last block id = 1 (in the start)
		//printf("space0: %d\n",hp_info->available_space);

      	BF_Block_SetDirty(hp_info->last_block);
    }
    
    /*Get data from the last_block (the newest) and copy block_info in the end of the block*/
    data = BF_Block_GetData(hp_info->last_block);
    block_info = (HP_block_info*)data;// + BF_BLOCK_SIZE - sizeof(block_info);    			//***************
    
    /*Insert records based to available space*/
	
    if(record_size <= hp_info->available_space){
      
      	memcpy(data + sizeof(block_info) + (block_info->block_records * record_size), &record, record_size);   	// *********** old:data + (BF_BLOCK_SIZE - hp_info->available_space), (block_info->block_records * record_size)
		
		Record* rec =(Record*)(data + sizeof(block_info) +(block_info->block_records * record_size)); //+ (block_info->block_records * record_size);
		printRecord(*rec);
		block_info->block_records++;                                                     	// Records inside current block +1
      	hp_info->available_space = hp_info->available_space - record_size;                	// Update available space
		//printf("\nspace1: %d\n",hp_info->available_space);	
	  	hp_info->file_records++;                                                          	// Records inside file(total) +1
      
      	BF_Block_SetDirty(hp_info->last_block);
    }
    else{

    	error = BF_UnpinBlock(hp_info->last_block);                                       	// Unpin the previous block
      	if( error != BF_OK) { BF_PrintError(error); return -1; }
		//BF_Block_Destroy(&hp_info->last_block); havent used it yet
      
		BF_Block *new_block = NULL;                                                       	// Create new block
		BF_Block_Init(&new_block);
		error = BF_AllocateBlock(file_desc, new_block); 
    	if( error != BF_OK) { BF_PrintError(error);  return -1; }
      
		hp_info->last_block = new_block;                                                  	// Now the last block, is the new block created
		hp_info->available_space = BF_BLOCK_SIZE - sizeof(block_info);                 	  	// Available space like in the start
		//printf("s:%d\n", hp_info->available_space);
		hp_info->last_block_id++;
		//printf("s1:%d\n", hp_info->available_space);
      	block_info->next_block = new_block;
		//printf("s2:%d\n", hp_info->available_space);	
      	char* data2 = BF_Block_GetData(new_block);                                   	// Get the new data
		//printf("s3:%d\n", hp_info->available_space);

		block_info = (HP_block_info*)data2;// + BF_BLOCK_SIZE - sizeof(block_info);    			// *************

		//memcpy(data + BF_BLOCK_SIZE - sizeof(block_info), block_info, sizeof(block_info));
		//printf("s4:%d\n", hp_info->available_space);
		//printf("wtf:%p\n",data2);
		//printf("wtf_rec:%p\n", &record);
		block_info->block_records = 0;                                                    	// New block, zero records

		memcpy(data2 + sizeof(block_info), &record, record_size);  	// **************
		Record* rec =(Record*)(data2 + sizeof(block_info)); //+ (block_info->block_records * record_size);
		printRecord(*rec);
		//printf("s5:%d\n", hp_info->available_space);
		block_info->block_records++;                                                      	// Update block records
		hp_info->file_records++;                                                          	// File records
		hp_info->available_space = hp_info->available_space - record_size;                	// And available space
		//printf("\nspace2: %d\n",hp_info->available_space);
			
		BF_Block_SetDirty(hp_info->last_block);
    }
    
    ///////// DEBUG ///////////////
    
    int bla;
    BF_GetBlockCounter(file_desc,&bla);
    printf("block_records:%d\n", block_info->block_records);
    // printf("file_records: %d\n",hp_info->file_records);
    // printf("blocks: %d\n",bla);
    // printf("blocks_to_write: %d\n",bla-1);
    // printf("last_block_id: %d\n", hp_info->last_block_id);
	// printf("data address: %lu\n",(unsigned long)data);
	// printf("block address: %lu\n",(unsigned long)hp_info->last_block);
	
	//printf("byte difference:%ld\n",(data - (char*)hp_info->last_block)* sizeof(int));
    
    ////////// END DEBUG ////////////
    
    return hp_info->last_block_id;

}

int HP_GetAllEntries(int file_desc, HP_info* hp_info, int value){    
    BF_ErrorCode error;
    
    Record* record;
    BF_Block *block = NULL;                                      
    BF_Block_Init(&block);
    error = BF_AllocateBlock(file_desc, block); 
    if( error != BF_OK) { BF_PrintError(error);  return -1; }
    
    int blocks_read = 0;
    int blocks_number;
    error = BF_GetBlockCounter(file_desc,&blocks_number);
    if( error != BF_OK) { BF_PrintError(error);  return -1; }

    HP_block_info* block_info;
    
    for(int i = 0; i < blocks_number; i++){

        error = BF_GetBlock(file_desc, i, block);
        if( error != BF_OK) { BF_PrintError(error);  return -1; }
        char* data = BF_Block_GetData(block);
        block_info = (HP_block_info*)data;
        record =(Record*)(data + sizeof(block_info)); //*************
        for(int j = 0; j < block_info->block_records; j++) {
        	//if(record[j].id == value) {
        		//printRecord(*record );
          	//}	
          
        }
		blocks_read++;
		
		error = BF_UnpinBlock(block);
  		if( error != BF_OK) { BF_PrintError(error); return -1; }
    }
	
    return blocks_read;
}
