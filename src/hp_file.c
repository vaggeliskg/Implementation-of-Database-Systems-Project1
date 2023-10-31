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
    info->last_block_id = 0;                      //MIDENIZONTAI KAI GINONTAI NULL TA PEDIA AUTA WTF NIGGA

    
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
    
    /*Init and allocate blocks*/
    int blocks_num; 
    error = BF_GetBlockCounter(file_desc, &blocks_num); 
    if( error != BF_OK) { BF_PrintError(error);  return -1; }
    
    HP_block_info* block_info;
    char* data;

    if(hp_info->last_block_id == 0){

      BF_Block *block = NULL;                                       //create a block for a records(needs check)
      BF_Block_Init(&block);
      error = BF_AllocateBlock(file_desc,block); 
      if( error != BF_OK) { BF_PrintError(error);  return -1; }

      hp_info->last_block = block;
      hp_info->available_space = BF_BLOCK_SIZE - sizeof(HP_block_info);
      hp_info->last_block_id++;

      BF_Block_SetDirty(hp_info->last_block);
    }
    
    data = BF_Block_GetData(hp_info->last_block);
    block_info = (HP_block_info*)data ;//+ BF_BLOCK_SIZE - sizeof(HP_block_info);    //***************
    
    // Insert records
    if(sizeof(record) < hp_info->available_space){
      //printf("no0\n");
      memcpy(data + sizeof(record), &record, sizeof(record));   // *********** old:data + (BF_BLOCK_SIZE - hp_info->available_space)
      //printf("no1\n");
      block_info->block_records++;
      //printf("no2\n");
      hp_info->available_space = hp_info->available_space - sizeof(record);
      //printf("no3\n");
      hp_info->file_records++;
      //printf("no4\n");
      BF_Block_SetDirty(hp_info->last_block);
    }
    else{
      block_info->block_records = 0;
      //printf("yes0\n");
      error = BF_UnpinBlock(hp_info->last_block);
      if( error != BF_OK) { BF_PrintError(error); return -1; }
      
      //printf("yes1\n");
      BF_Block *next_block = NULL;                                       //create a block for a records(needs check)
      BF_Block_Init(&next_block);
      error = BF_AllocateBlock(file_desc, next_block); 
      if( error != BF_OK) { BF_PrintError(error);  return -1; }
      //printf("yes2\n");
      hp_info->last_block = next_block;
    //   block_info->next_block = next_block;
      hp_info->available_space = BF_BLOCK_SIZE - sizeof(HP_block_info);
    //   hp_info->last_block_id++;
      //printf("yes3\n");
      char* data2 = BF_Block_GetData(next_block);
      block_info = (HP_block_info*)data2 ;//+ BF_BLOCK_SIZE - sizeof(HP_block_info);           // *************
      memcpy(data2 + sizeof(record), &record, sizeof(record)); // **************
      //printf("yes4\n");
      block_info->block_records ++;
      hp_info->file_records++;
      //printf("yes5\n");
      hp_info->available_space - sizeof(record);
      //printf("yes6\n");
      // hp_info->available_space++;
      // printf("yes7\n");
      BF_Block_SetDirty(hp_info->last_block);
    }
    
    int bla;
    BF_GetBlockCounter(file_desc,&bla);
    printf("block_records:%d\n", block_info->block_records);
    printf("file_records: %d\n",hp_info->file_records);
    printf("blocks: %d\n",bla);
    return hp_info->last_block_id;

}

int HP_GetAllEntries(int file_desc,HP_info* hp_info, int value){    
    return -1;
}

