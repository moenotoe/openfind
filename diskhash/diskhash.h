#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#define MAX_KEY_LEN              256
#define VALUE_MAX              4194304
#define MAX_ENTRIES            1000000
#define FNV_OFFSET_BASIS     0x811C9DC5
#define FNV_PRIME            0x01000193
#define TABLE_SIZE              90667
#define BUFFER_SIZE              1024
#define INDEX_FILE          "indexFile.txt"
#define DATA_FILE           "dataFile.txt"
#define TMP_FILE            "tmpFile.txt"

#define SUCCESS                   0

#define UNKNOWN_ERR              -1
#define FILE_NOT_FOUND           -2
#define INIT_INDEX_FAILED        -3
#define MEMORY_ERROR             -4
#define POSITION_ERROR           -5
#define WRITTEN_FAIL             -6
#define REWRITE_FAIL             -7
#define DELETE_FAIL              -8
#define COMMAND_NOT_FOUND        -9
#define PARA_ERROR               -10
#define FSEEK_ERROR              -11
#define CLEAN_FAIL               -12
#define ADD_UPDATE_ERR           -13
#define EXPORT_FAIL              -14
typedef enum
{
    INT,
    STR,
    BIN
}dataType;


//return value
typedef struct{
    dataType type;
    void* data;
    long int dataSize;
}Value;


typedef struct {
    char key[MAX_KEY_LEN];    
    long valSize;
    unsigned long offset;
    dataType type;
    bool isDelete;
} IndexFile;   


int add(const char* fileName);
int addOrUpdate(char* key, char* value, dataType type);
int delete(char* key);
Value* query(const char* key);
int import(char* fileName, dataType type);
int export(char* key,  char* storeFile);
int test();


//help function
int rewrite2IndexFile(IndexFile** indexTable, int indexTableSize);
char* getDataAns(const int offset, const int valSize);
unsigned int hashFunction(const char* key);
int cleanContent();
int hashClean();


