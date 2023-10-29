#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName){
    BF_CreateFile(fileName);          // create en empty file
    printf("create:filename:%d\n", *fileName);
    
    int file;                         // file reco
    BF_OpenFile(fileName, &file);
    printf("create_file:filedesc:%d\n",file);

    BF_Block *block = NULL;
    BF_Block_Init(&block);            // it may not be necessary
    BF_AllocateBlock(file,block);     //allocate the first block of the file


    
    //HP_info *block_info;
    
    HP_info* jj_gamiesai;
    //HP_info *pinfo;
    jj_gamiesai = (HP_info*) BF_Block_GetData(block);
    jj_gamiesai->file_records = 12;
    //memcpy(block, &jj_gamiesai, sizeof(HP_info));
    //pinfo = (HP_info*)block;
    
    //block_info = (HP_block_info *) block + BF_BLOCK_SIZE - sizeof(HP_block_info);
    
    //pinfo->file_records = 4;


    BF_Block_SetDirty(block);
    //BF_UnpinBlock(block);
    //BF_Block_Destroy(&block);
    BF_CloseFile(file);
    //printf("closed--1\n");
	
    // DEBUG
    // printf("1.%d\n", block_info->number_of_records);                 
    // printf("3.%ld\n", sizeof(HP_block_info));
    // printf("5.%p____%p\n", block, block_info);
    // printf("1.%p\n", data); 
    // printf("1.%p\n", &block_info->number_of_records);
    //printf("FILE: %d\n",file);
	//printf("1.%p\n", block);

    return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){    
  printf("open:filename:%d\n", *fileName);

	BF_OpenFile(fileName, file_desc);
  printf("open:filedesc:%d\n", *file_desc);

	//printf("opened--1\n");
	BF_Block* block = NULL;
	BF_Block_Init(&block);
	
	BF_GetBlock(*file_desc, 1 , block);
	
	void* data;
	data = BF_Block_GetData(block);
	
	HP_info* hp_info;
  hp_info = (HP_info*)block;
  
	//memcpy(hp_info, data, sizeof(data));

	//BF_AllocateBlock(*file_desc, block);
	//HP_block_info *block_info;
	
	//block_info = (HP_block_info *) block + BF_BLOCK_SIZE - sizeof(HP_block_info);
	
	//hp_info = (HP_info*)block;
	//pinfo->file_records = 8;
  	//pinfo = (HP_info*) block;

	//void* data;
  	//HP_block_info kati;

	//block_info = &kati;


	//BF_GetBlock(*file_desc, block_id, block);
	//data = BF_Block_GetData(block);
	//pinfo = (HP_info*)block;

	//printf("blaaa: %d\n", pinfo->file_records);
	//printf("2.%p\n", block);
	//pinfo->file_records += block_info->block_records;
	printf("blaaa: %d\n", hp_info->file_records);


	//info = (HP_info*) block + BF_BLOCK_SIZE - sizeof(HP_block_info);

	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
	
	return hp_info;

 }


int HP_CloseFile(int file_desc,HP_info* hp_info ){
    
    //printf("FILEEEE: %d\n",file_desc);
    BF_ErrorCode err = BF_CloseFile(file_desc);
    //printf("closed--2\n");
    if( err =! BF_OK) {
     BF_PrintError(err);
     printf("\nerror_code:%d\n", err);
      return -1;
    }
    else{
      printf("gooood\n");
      return 0;
    } 
}

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
    return -1;
}

int HP_GetAllEntries(int file_desc,HP_info* hp_info, int value){    
    return -1;
}

