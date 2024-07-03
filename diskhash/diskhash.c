#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include "diskhash.h"

int add(const char* fileName){
    FILE* file=fopen(fileName,"r+");
    if(file==NULL){
        return FILE_NOT_FOUND;
    }
    int count=0;
    char key[MAX_KEY_LEN];
    char value[BUFFER_SIZE];
    while (fscanf(file, "%256s\t%1024[^\n]\n", key, value)==2) {
        int state=addOrUpdate(key,value,STR);
        if(state!=SUCCESS){
            return ADD_UPDATE_ERR;
        }  
        count++;
    }
    fclose(file);
    printf("OK %d\n",count);
    return SUCCESS;
}

int import(char* fileName, dataType type){

    FILE* importFile=fopen(fileName,"r+b");

    if(importFile==NULL){
        printf("import file not found\n");
        return FILE_NOT_FOUND;
    }

    char* key=fileName;
    fseek(importFile, 0, SEEK_END);
    long fileSize=ftell(importFile);
    fseek(importFile, 0, SEEK_SET);

 
    char* importData=(char*)malloc(fileSize + 1);
    if (importData==NULL) {
        perror("Failed to allocate memory");
        fclose(importFile);
        return MEMORY_ERROR;
    }    
    importData[0]='\0';
    size_t bytesRead=fread(importData, 1, fileSize, importFile);
    importData[bytesRead]='\n';
  
    fclose(importFile);

    int state=addOrUpdate(key, importData, type);
    if(state!=SUCCESS){
        return ADD_UPDATE_ERR;
    }
    printf("OK %s %ld" ,fileName, fileSize);
    
    return SUCCESS;    
}

int export(char* key, char* storeFile){
    FILE* targetFile=fopen(storeFile,"a+b");
    FILE* indexFile=fopen(INDEX_FILE,"r+b");
    FILE* dataFile=fopen(DATA_FILE,"r+b");
    if(targetFile==NULL||indexFile==NULL||dataFile==NULL){
        return FILE_NOT_FOUND;
    }
    
    IndexFile indexObj;
    long hashVal=hashFunction(key);
    fseek(indexFile,hashVal*sizeof(IndexFile),SEEK_SET);
    fread(&indexObj, sizeof(IndexFile), 1, indexFile);
    if(strcmp(indexObj.key,key)!=0){
        return UNKNOWN_ERR;
    }
    
    char* tmpChar=getDataAns(indexObj.offset,indexObj.valSize);
    //append to storefile
    fseek(targetFile, 0, SEEK_END);
    fwrite(tmpChar, sizeof(char), indexObj.valSize+1, targetFile);
    fseek(targetFile, 0, SEEK_END);
    long fileSize=ftell(targetFile);

    fclose(targetFile);
    fclose(indexFile);
    fclose(dataFile);

    printf("OK %s %ld" ,storeFile, fileSize);
    return SUCCESS;
}

int delete(char* key){
    FILE* index=fopen(INDEX_FILE,"r+b");
    long hashVal=hashFunction(key);
    
    IndexFile indexfile;
    fseek(index,hashVal*sizeof(IndexFile),SEEK_SET);
    if (fread(&indexfile, sizeof(IndexFile), 1, index)!=1) {
        perror("key not found");
        fclose(index);
        return UNKNOWN_ERR;
    }
    if(strcmp(indexfile.key,"")==0){
        printf("failure\n");
        return DELETE_FAIL;
    }

    indexfile.isDelete=true;

    fseek(index, hashVal * sizeof(IndexFile), SEEK_SET);
    if (fwrite(&indexfile, sizeof(IndexFile), 1, index)!=1) {
        perror("Failed to write to the file");
        fclose(index);
        return WRITTEN_FAIL;
    }
    fclose(index);
    return SUCCESS;
}

int addOrUpdate(char* key, char* value, dataType type){
    int hashValue=hashFunction(key);
    //the key should be stored into offset
    FILE* indexFile=fopen(INDEX_FILE,"r+b");
    if(indexFile==NULL){
        return FILE_NOT_FOUND;
    }
   
     //initial point
    fseek(indexFile,hashValue*sizeof(IndexFile),SEEK_SET);
    
    IndexFile existingEntry;
 
    while(1){
        fread(&existingEntry, sizeof(IndexFile), 1, indexFile);

        if(strcmp(existingEntry.key,"")==0){
            break;
        }
        //same key
        if(strcmp(existingEntry.key, key)==0){
            char* dataAns=getDataAns(existingEntry.offset,existingEntry.valSize);

            if(strcmp(dataAns,value)==0){
                return SUCCESS;
            }
            FILE* dataFile=fopen(DATA_FILE, "a+b");
            if(dataFile==NULL){
                return FILE_NOT_FOUND;
            }
        
            fseek(dataFile,0,SEEK_END);
            size_t length=strlen(value)+1;
            fwrite(value, sizeof(char), length, dataFile);
            char extra='\n';
            if (fwrite(&extra, sizeof(char), 1, dataFile)!=1) {
                perror("Failed to write the  newline to the file");
                fclose(dataFile);
                return WRITTEN_FAIL;
            }       

            existingEntry.type=type;
            existingEntry.valSize=strlen(value)+1;
            existingEntry.offset=ftell(dataFile);
            existingEntry.isDelete=false;
            fclose(dataFile);
            //update data to index file
            fseek(indexFile,hashValue*sizeof(IndexFile),SEEK_SET);
            
            if (fwrite(&existingEntry, sizeof(IndexFile), 1, indexFile)!=1) {
                perror("failed to write to the index file");
                fclose(indexFile);
                return WRITTEN_FAIL;
            }
                


            fclose(indexFile);
            return SUCCESS;
        }else{
            //rehash;
            hashValue=(hashValue+1)%TABLE_SIZE;;
            fseek(indexFile,hashValue*sizeof(IndexFile),SEEK_SET);
        }
    }

    fclose(indexFile);

    //not exist:
    //1. Write into the data file
    FILE* dataFile=fopen(DATA_FILE,"a+b");
    long dataOffset=ftell(dataFile);
    size_t length=strlen(value);
    
    if (fwrite(value, sizeof(char), length, dataFile)!=length) {
        perror("Failed to write to the file");
        fclose(dataFile);
        return WRITTEN_FAIL;
    }
    dataOffset=ftell(dataFile);

    char extra='\n';
    if (fwrite(&extra, sizeof(char), 1, dataFile)!=1) {
        perror("Failed to write the  newline to the file");
        fclose(dataFile);
        return WRITTEN_FAIL;
    }
    
    
    dataOffset=ftell(dataFile);
 

    fclose(dataFile);

    IndexFile entry;
    strcpy(entry.key,key);
    entry.offset=dataOffset;
    entry.type=type;
    entry.valSize=strlen(value);
    entry.isDelete=false;

    FILE* indexFileWrite=fopen(INDEX_FILE,"r+b");
    if(indexFileWrite==NULL){
        return FILE_NOT_FOUND;
    }

    
    if (fseek(indexFileWrite, hashValue*sizeof(IndexFile), SEEK_SET)!=0) {
        perror("Failed to seek to position");
        fclose(indexFileWrite);
        return WRITTEN_FAIL;
    }

    if(fwrite(&entry, sizeof(IndexFile), 1, indexFileWrite)!=1) {
        perror("Failed to write to the file");
        fclose(indexFileWrite);
        return WRITTEN_FAIL;
    }

    IndexFile read;
    fseek(indexFileWrite, hashValue*sizeof(IndexFile), SEEK_SET);
    fread(&read,sizeof(IndexFile),1,indexFile);

    fclose(indexFileWrite);
    return SUCCESS;

}
char* getDataAns(const int offset, const int valSize){
    char* ans=(char*)malloc(valSize+1);
    if(ans==NULL){
        return NULL;
    }
    FILE* file=fopen(DATA_FILE,"a+b");
    if(file==NULL){
       
        free(ans);
        return NULL;
    }
    fseek(file, offset-(valSize+1), SEEK_SET);
    fread(ans, 1, valSize+1, file);
    fclose(file);
    return ans;
}

Value* query(const char* key){

    Value* ans=(Value*)malloc(sizeof(Value));
    if(ans==NULL){
        return NULL;
    }
    long hashVal=hashFunction(key);

    FILE* readIndexFile=fopen(INDEX_FILE,"r+b");
    if(readIndexFile==NULL){
        return NULL;
    }
    
    if(fseek(readIndexFile,hashVal*sizeof(IndexFile),SEEK_SET)!=0){
        perror("Failed to seek to position");
        fclose(readIndexFile);
        return NULL;
    }
    IndexFile existingEntry;


    while(1){
        fread(&existingEntry, sizeof(IndexFile), 1, readIndexFile);

        if(existingEntry.isDelete==true){
            printf("key not exist\n");
            return NULL;
        }
       
       
        if(strcmp(existingEntry.key,"")==0){
            printf("key not found\n");
            return NULL;
        }else if(strcmp(existingEntry.key,key)==0){
            char* strAns=getDataAns(existingEntry.offset, existingEntry.valSize);
            if(existingEntry.type==STR){
                ans->type=STR;
                ans->dataSize=existingEntry.valSize;
                ans->data=strAns;
                return ans;
            }else if(existingEntry.type==BIN){
                ans->type=BIN;
                ans->dataSize=existingEntry.valSize;
                ans->data=strAns;
                return ans;
            }else if(existingEntry.type==INT){
                ans->type=INT;
                long* intAns=malloc(sizeof(long));
                if(intAns==NULL){
                    return NULL;
                }
                *intAns=atol(strAns);
                return ans;
            }
        }else{
            hashVal=(hashVal+1)%TABLE_SIZE;
            fseek(readIndexFile,hashVal*sizeof(IndexFile),SEEK_SET);
        }   
    }
        
    fclose(readIndexFile);

    return SUCCESS;
}


int hashClean(){
    FILE* indexFile=fopen(INDEX_FILE ,"r+b");
    FILE* dataFile=fopen(DATA_FILE,"r+b");
    FILE* tmpdataFile=fopen(TMP_FILE,"w+b");
    //if del==1, return
    if(indexFile==NULL||dataFile==NULL||tmpdataFile==NULL){
        if (indexFile) fclose(indexFile);
        if (dataFile) fclose(dataFile);
        if (tmpdataFile) fclose(tmpdataFile);
        return FILE_NOT_FOUND;
    }

   IndexFile entry;
   while(fread(&entry, sizeof(IndexFile), 1, indexFile)==1){
      if((strcmp(entry.key,"")!=0) && entry.isDelete==false){
        //move the val to new file
            char* data=getDataAns(entry.offset,entry.valSize);
            fwrite(data, sizeof(char), entry.valSize+1, tmpdataFile);
            char extra='\n';
            fwrite(&extra, sizeof(char), 1, tmpdataFile);
            //update the offset
            long offset=ftell(tmpdataFile);
            entry.offset=offset;

            fseek(indexFile,-sizeof(IndexFile), SEEK_CUR);
            fwrite(&entry, sizeof(IndexFile),1, indexFile);
            free(data);
        }

    }   
    fclose(indexFile);
    fclose(dataFile);
    fclose(tmpdataFile);
    rename(TMP_FILE, DATA_FILE);
   

    return SUCCESS;
}



unsigned int hashFunction(const char* key) {
    unsigned int hashValue=FNV_OFFSET_BASIS;
    int charToInt;

    while ((charToInt=*key++)) {
        hashValue=hashValue*FNV_PRIME; 
        hashValue^=charToInt;  
    }
    return hashValue%TABLE_SIZE;
}

int cleanContent(){
    FILE* file1=fopen(INDEX_FILE, "w");
    FILE* file2=fopen(DATA_FILE, "w");
    if (file1==NULL||file2==NULL) {
        perror("Failed to open file");
        return FILE_NOT_FOUND;
    }
    fclose(file1);    
    fclose(file2);
    return SUCCESS;
}
