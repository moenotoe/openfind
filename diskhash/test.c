#include "diskhash.h"

int main(int argc, char* argv[]){
    int state=0;
    if(argc<2){
        printf("parameter error");
        return PARA_ERROR;
    }
    const char* mode=argv[1];
    if (strcmp(mode, "add")==0&&argc==3) {
        state=add(argv[2]);
        if (state!=SUCCESS) return ADD_UPDATE_ERR;
    } else if (strcmp(mode, "query")==0&&argc==3) {
        state=query(argv[2]);
        if (state!=SUCCESS) return ADD_UPDATE_ERR;
    } else if (strcmp(mode, "del")==0&&argc==3) {
        state=delete(argv[2]);
        if (state!=SUCCESS) return DELETE_FAIL;
    } else if (strcmp(mode, "import")==0&&argc==4) {
        state=import(argv[2],atoi(argv[3]));
        if (state!=SUCCESS) return ADD_UPDATE_ERR;
    } else if (strcmp(mode, "export")==0&&argc==4) {
        state=export(argv[2], argv[3]);
        if (state!=SUCCESS) return EXPORT_FAIL;
    } else if(strcmp(mode, "clean")==0&&argc==2){
        state=hashClean();
        if (state!=SUCCESS) return HASH_CLEAN_FAIL;
    }else {
        printf("Invalid mode or wrong number of arguments\n");
        return COMMAND_NOT_FOUND;
    }
    

    return SUCCESS;
}

int test(){
    cleanContent();
    addOrUpdate("ken","chi",STR);
    query("ken");
    addOrUpdate("ken","jamesHad",STR);
    query("ken");
    delete("ken");
    query("ken");
    import("exam.txt", STR);
    query("exam.txt");
    add("addfile.txt");
    query("ken");
    export("ken","example.txt");
    hashClean();
    
    return SUCCESS;
}
