
#include "Buffer/StaticBuffer.h"
#include "Buffer/BlockBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <cstring>
using std::cout;
using std::endl;

//STAGE 1
void stage1(){
  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer,7000);
  char message[]="hello";
  memcpy(buffer+20,message,6);
  Disk::writeBlock(buffer,7000);
  
  unsigned char buffer2[BLOCK_SIZE];
  char message2[6];
  Disk::readBlock(buffer2,7000);
  memcpy(message2,buffer2+20,6);
  std::cout<<message2;
  // return 0;
 // return FrontendInterface::handleFrontend(argc, argv);
}


//STAGE 2
void stage2(){
  // create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);
  
  for (int i=0;i<relCatHeader.numEntries;i++) {
    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog
    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
    
    for (int j=0;j<attrCatHeader.numEntries;j++) {

      // declare attrCatRecord and load the attribute catalog entry into it
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBuffer.getRecord(attrCatRecord,j);
      if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal)==0) {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
        printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
      
      
    }
    printf("\n");
  }

} 

//STAGE 2 Exercise

void stage2ex1(){
  // Create objects for the relation catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);

  HeadInfo relCatHeader;

  // Load the header of the relation catalog block
  relCatBuffer.getHeader(&relCatHeader);

  // Iterate through all relations in the relation catalog
  for (int i = 0; i < relCatHeader.numEntries; i++) {
  
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBuffer.getRecord(relCatRecord, i);
                                                                                                                                                                                                             printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    int currentBlock = ATTRCAT_BLOCK;

    // Traverse through all blocks of the attribute catalog
    while (currentBlock != -1) {
      RecBuffer attrCatBuffer(currentBlock);
      HeadInfo attrCatHeader;
      attrCatBuffer.getHeader(&attrCatHeader);
      
     // Iterate through all entries in the current block
      for (int j = 0; j < attrCatHeader.numEntries; j++) 
      { 
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];     //Get the attribute catalog record
       attrCatBuffer.getRecord(attrCatRecord, j);
        

        // Check if the attribute belongs to the current relation
        if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, 
                   attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal) == 0) {
          const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
          printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
        }
      }
       // Move to the next block in the attribute catalog
      currentBlock = attrCatHeader.rblock;
    }

    printf("\n");
  }

 // return 0;
} 
     
void stage2ex2(){
   RecBuffer attrCatBuffer(ATTRCAT_BLOCK);
   HeadInfo attrCatHeader;
   attrCatBuffer.getHeader(&attrCatHeader);
  
   
        // Iterate through all entries in the current block
        for (int j = 0; j < attrCatHeader.numEntries; j++) {
          Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
          attrCatBuffer.getRecord(attrCatRecord, j);
           
          if (strcmp (attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,"Students") == 0 && strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Class")==0) {
        strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Batch");
        attrCatBuffer.setRecord(attrCatRecord,j);
        break;
     }
     if(j==attrCatHeader.numSlots-1){
			j=-1;
			attrCatBuffer=RecBuffer(attrCatHeader.rblock);
			attrCatBuffer.getHeader(&attrCatHeader);
		}
	}
}

void stage3(){
  for(int i=0;i<=2;i++){  //stage3ex1 including relation student
   RelCatEntry relCatBuf;
   RelCacheTable::getRelCatEntry(i,&relCatBuf);
   cout<<"Relation: "<<relCatBuf.relName<<"\n";
     for(int j=0;j<relCatBuf.numAttrs;j++){
      AttrCatEntry attrCatBuf;
      AttrCacheTable::getAttrCatEntry(i,j,&attrCatBuf);
      const char *attrType=attrCatBuf.attrType==NUMBER?"NUM":"STR";
      cout<<" "<<attrCatBuf.attrName<<": "<<attrType<<"\n";
     }
    cout<<"\n";
   }
  }
  
int main(int argc, char *argv[]){
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;
  //stage2ex1();
  //stage2ex2();
  //stage2ex1();
  
   return FrontendInterface::handleFrontend(argc,argv);
  return 0;
 } 


