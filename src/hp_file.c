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
    //error =  BF_CreateFile(fileName);           // create en empty file
    //if( error != BF_OK) { BF_PrintError(error); return -1; }
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
    info->file_records = 69;

    // START TESTING
    HP_info* p;
    p =(HP_info*) BF_Block_GetData(block);
    printf("create:file_records %d\n", p->file_records);
    // STOP TESTING
    
    BF_Block_SetDirty(block);
    error = BF_UnpinBlock(block);
    if( error != BF_OK) { BF_PrintError(error); return -1; }

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

  printf("number_of_blocks:%d\n", block_num);

  error = BF_GetBlock(*file_desc, block_num - 1 , block);
  if( error != BF_OK) { BF_PrintError(error);  exit(EXIT_FAILURE); }
		
	HP_info* hp_info;
  hp_info = (HP_info*)BF_Block_GetData(block);

	printf("open:file_records: %d\n", hp_info->file_records);


	error = BF_UnpinBlock(block);
  if( error != BF_OK) { BF_PrintError(error);  exit(EXIT_FAILURE); }

	//BF_Block_Destroy(&block);
	
	return hp_info;

 }


int HP_CloseFile(int file_desc,HP_info* hp_info ){
    BF_Block *block = NULL;
    BF_Block_Init(&block);   
    BF_ErrorCode err2 = BF_GetBlock(file_desc, 0 , block);
    BF_PrintError(err2);
    BF_UnpinBlock(block);
    //printf("FILEEEE: %d\n",file_desc);
    BF_ErrorCode err = BF_CloseFile(file_desc);
    //printf("closed--2\n");
    if( err != BF_OK) {
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

