#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>


#include "storage_mgr.h"
#include "dberror.h"

void initStorageManager(void)
{
    printf("Storage Manager initialized\n");
}

/* Creates a single page file with the input file name and return OK if success. */

RC createPageFile(char *fileName) {

    FILE *ftr;
    char *size = (char *) malloc(PAGE_SIZE); /* First page of data */
    ftr = fopen(fileName, "w+");
    fwrite(size,sizeof(char),PAGE_SIZE,ftr);
    fclose(ftr);
    return RC_OK;
}

/* open page file */

RC openPageFile(char *fileName, SM_FileHandle *fHandle) {

    /* This is to check if file exists */
    if (access(fileName, F_OK) != -1) {
        /* file exists */
        int size;
        FILE *fp;
        fp = fopen(fileName, "r");
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fHandle->fileName = fileName;
        fHandle->totalNumPages = (int) size / PAGE_SIZE;
        fHandle->curPagePos = 0;
        fHandle->mgmtInfo=fp;
        return RC_OK;
    } 

    else {
        /* file doesn't exist */
        return RC_FILE_NOT_FOUND;
    }
}

/* close file */

RC closePageFile(SM_FileHandle *fHandle) {

    char *fileName = fHandle->fileName;
    /* This is to check if file exists */
    if (access(fileName, F_OK) != -1) { 
        /* file exists */        
        FILE *fp = fopen(fileName, "r");
        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
        return RC_OK;

    } 

    else {
        return RC_FILE_NOT_FOUND;
    }
}

/* destroy file */

RC destroyPageFile(char *fileName) {
    int result;
    result = remove(fileName);
    
    if (result == 0) {
        printf("File destroyed successfully.\n");
        return RC_OK;
    } else
        return RC_FILE_NOT_FOUND;

}

/* Write block Functions */

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	int flagSeek;
	size_t writeBlock;
	/* checking pageNum is less than total number of pages */
	
    if(pageNum>(fHandle->totalNumPages)||pageNum<0){
		return RC_WRITE_FAILED;
    }
	flagSeek=fseek(fHandle->mgmtInfo,pageNum+1*PAGE_SIZE*sizeof(char),SEEK_SET);
	
    if(flagSeek==0){
		writeBlock=fwrite(memPage,sizeof(char),PAGE_SIZE,fHandle->mgmtInfo);
		fHandle->curPagePos=pageNum;
		return RC_OK;
	}
	
    else{
		return RC_WRITE_FAILED;
	}
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return writeBlock(fHandle->curPagePos,fHandle,memPage);
}

extern RC appendEmptyBlock (SM_FileHandle *fHandle){
	int flagSeek;
	size_t wSizeBlock;
	SM_PageHandle dummyBlock;
	dummyBlock=(SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));
	flagSeek=fseek(fHandle->mgmtInfo,(fHandle->totalNumPages+1)*PAGE_SIZE*sizeof(char),SEEK_END);
	
    if(flagSeek==0){
		wSizeBlock=fwrite(dummyBlock,sizeof(char),PAGE_SIZE,fHandle->mgmtInfo);
		fHandle->totalNumPages=fHandle->totalNumPages+1;
		rewind(fHandle->mgmtInfo);  /* sets position to the startinf og the file */
		/* set position of file to starting again */
		fseek(fHandle->mgmtInfo,(fHandle->totalNumPages)*PAGE_SIZE*sizeof(char),SEEK_SET);
		free(dummyBlock);
		return RC_OK;
	}
	
    else{
		free(dummyBlock);
		return RC_WRITE_FAILED;
	}
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
	int numPages,value=0;
	
    if(fHandle->totalNumPages<numberOfPages){
		numPages=numberOfPages-fHandle->totalNumPages;
	}
	
    while(value<numPages){
		appendEmptyBlock(fHandle);
		value++;	
	}
    return RC_OK;
}

/* Read Block Functions */

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
        if (pageNum >= fHandle->totalNumPages || pageNum < 0){
            return RC_READ_NON_EXISTING_PAGE;
        }
        
        if(fHandle->mgmtInfo!=NULL){
            fseek(fHandle->mgmtInfo, (pageNum+1)*PAGE_SIZE *sizeof(char), SEEK_SET);
            fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);
            fHandle->curPagePos = pageNum;
            return RC_OK;
        }
        
        else{
            return RC_FILE_NOT_FOUND;
        }
}


int getBlockPos(SM_FileHandle *fHandle){
    return fHandle->curPagePos;
}


RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(0, fHandle, memPage);
}

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(fHandle->curPagePos - 1, fHandle, memPage);
}

RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(fHandle->curPagePos + 1, fHandle, memPage);
}

RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}