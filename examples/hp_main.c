#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"

#define RECORDS_NUM 1000 // you can change it if you want // mhn jexasoume na to ksanavaloume 1000
#define FILE_NAME "data.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
  BF_Init(LRU);
  printf("main:filename:%s\n", FILE_NAME);
  HP_CreateFile(FILE_NAME);
  int file_desc;
  printf("main:file_desc:%d\n", file_desc);
  HP_info* hp_info2=HP_OpenFile(FILE_NAME, &file_desc);
  
  Record record;
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    printf("\n==>LOOP:%d\n", id+1);
    record = randomRecord();
    HP_InsertEntry(file_desc,hp_info2, record);
  }
  int block_num;
  BF_GetBlockCounter(file_desc, &block_num);
  
  printf("number_of_blocks:%d\n", block_num);

  printf("RUN PrintAllEntries\n");
  int id = rand() % RECORDS_NUM;
  printf("\nSearching for: %d",id);
  HP_GetAllEntries(file_desc,hp_info2, id);

  HP_CloseFile(file_desc,hp_info2);
  BF_Close();
}
