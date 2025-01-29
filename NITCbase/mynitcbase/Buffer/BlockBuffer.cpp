#include "BlockBuffer.h"
#include<iostream>
#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  this->blockNum=blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head) {
  //unsigned char buffer[BLOCK_SIZE];
  
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;   // return any errors that might have occured in the process
  }

/*
  // read the block at this.blockNum into the buffer
  if(Disk::readBlock(buffer,this->blockNum) !=SUCCESS){return FAILURE;};*/
  
  // populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->numSlots, bufferPtr + 24, 4);
  memcpy(&head->numEntries, bufferPtr + 16, 4);
  memcpy(&head->numAttrs, bufferPtr + 20, 4);
  memcpy(&head->rblock, bufferPtr + 12, 4);
  memcpy(&head->lblock, bufferPtr + 8, 4);
  memcpy(&head->pblock, bufferPtr + 4, 4);

  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }
  
  struct HeadInfo head;

  // get the header using this.getHeader() function
  if(this->getHeader(&head)!=SUCCESS){return FAILURE;}

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;
/*
  // read the block at this.blockNum into a buffer
  unsigned char buffer[BLOCK_SIZE];
  
  if(Disk::readBlock(buffer,this->blockNum)!=SUCCESS){return FAILURE;}*/
  

  int recordSize = attrCount * ATTR_SIZE;
  int slotMapSize= slotCount;
  unsigned char *slotPointer = bufferPtr+ HEADER_SIZE+slotMapSize+(recordSize*slotNum);

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

/*
Used to load a block to the buffer and get a pointer to it.
NOTE: this function expects the caller to allocate memory for the argument
*/
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
  // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
  
   if (bufferNum != E_BLOCKNOTINBUFFER)
  {
    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++)
    {
      StaticBuffer::metainfo[bufferIndex].timeStamp++;
    }
    StaticBuffer::metainfo[bufferNum].timeStamp = 0;
  }

  else {
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }

    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }

  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];

  return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
   int bufferNum=loadBlockAndGetBufferPtr(&bufferPtr);
   if(bufferNum !=SUCCESS){
    return bufferNum;
    }
   
    /* get the header of the block using the getHeader() function */
    HeadInfo head;
    BlockBuffer::getHeader(&head);

    // get number of attributes in the block.
    
    int attrCount=head.numAttrs;

    // get the number of slots in the block.
    int slotCount=head.numSlots;
    if(slotNum>=slotCount || slotNum<0){
     return E_OUTOFBOUND; }

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.

    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */
    int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer = bufferPtr + (HEADER_SIZE + slotCount + (recordSize * slotNum));
  memcpy(slotPointer, rec, recordSize);
  

    // update dirty bit using setDirtyBit()
    int ret = StaticBuffer::setDirtyBit(this->blockNum);
    if(ret!=SUCCESS){
     std::cout<<"There is error\n";
     exit(1);
    }

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */
       

    return SUCCESS;
}
/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret=loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret!=SUCCESS) {
    return ret;
  }
  struct HeadInfo head;
  // get the header of the block using getHeader() function
  BlockBuffer::getHeader(&head);
  int slotCount =head.numSlots;
  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;
  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  memcpy(slotMap,slotMapInBuffer,slotCount);
  return SUCCESS;
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    double diff;
    if(attrType==STRING){
    	diff=strcmp(attr1.sVal,attr2.sVal);
    }
    else{
    	diff=attr1.nVal-attr2.nVal;
    }
    if(diff>0)
         {return 1;}
    else if(diff==0)
          {return 0;}
    else
       {return -1;}
    
}




