
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
/*int main(int argc, char *argv[]) {
  //Initialize the Run Copy of Disk 
  Disk disk_run;
  // StaticBuffer buffer;
  // OpenRelTable cache;
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
  return 0;
  return FrontendInterface::handleFrontend(argc, argv);

}*/


//STAGE 2
/*int main(int argc, char *argv[]) {
  Disk disk_run;

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
    int  attrCatSlotIndex = 0;
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
      
      if(attrCatSlotIndex == attrCatHeader.numSlots-1){
      	attrCatSlotIndex=-1;
      	attrCatBuffer=RecBuffer(attrCatHeader.rblock);
      	attrCatBuffer.getHeader(&attrCatHeader);
      }else{
      attrCatSlotIndex++;}
    }
    printf("\n");
  }

  return 0;
} */

int main(int argc, char *argv[]) {
  Disk disk_run;

  // Create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);

  HeadInfo relCatHeader;

  // Load the header of the relation catalog block
  if (relCatBuffer.getHeader(&relCatHeader) != SUCCESS) {
    return FAILURE;
  }

  // Iterate through all relations in the relation catalog
  for (int i = 0; i < relCatHeader.numEntries; i++) {
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    // Get the relation catalog record
    if (relCatBuffer.getRecord(relCatRecord, i) != SUCCESS) {
      std::cerr << "Failed to load relation catalog record " << i << std::endl;
      continue;
    }                                                                                                                                                                                                         printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    int currentBlock = ATTRCAT_BLOCK;

    // Traverse through all blocks of the attribute catalog
    while (currentBlock != -1) {
      RecBuffer attrCatBuffer(currentBlock);
      HeadInfo attrCatHeader;

      // Load the header of the current attribute catalog block
      if (attrCatBuffer.getHeader(&attrCatHeader) != SUCCESS) {
        std::cerr << "Failed to load attribute catalog header for block " << currentBlock << std::endl;
        break;
     }
     // Iterate through all entries in the current block
      for (int j = 0; j < attrCatHeader.numEntries; j++) 
      { 
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];     //Get the attribute catalog record
        if (attrCatBuffer.getRecord(attrCatRecord, j) != SUCCESS) {
          std::cerr << "Failed to load attribute catalog record " << j << " in block " << currentBlock << std::endl;
          continue;
        }

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

  return 0;
} 
     
 
    
    
