#include "BlockBuffer.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  this->blockNum=blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

RecBuffer::RecBuffer() : BlockBuffer('R'){}
// call parent non-default constructor with 'R' denoting record block.

// call the corresponding parent constructor
IndBuffer::IndBuffer(char blockType) : BlockBuffer(blockType){}

// call the corresponding parent constructor
IndBuffer::IndBuffer(int blockNum) : BlockBuffer(blockNum){}

IndInternal::IndInternal() : IndBuffer('I'){}
// call the corresponding parent constructor
// 'I' used to denote IndInternal.

IndInternal::IndInternal(int blockNum) : IndBuffer(blockNum){}
// call the corresponding parent constructor

IndLeaf::IndLeaf() : IndBuffer('L'){} // this is the way to call parent non-default constructor.
// 'L' used to denote IndLeaf.

//this is the way to call parent non-default constructor.
IndLeaf::IndLeaf(int blockNum) : IndBuffer(blockNum){}

BlockBuffer::BlockBuffer(char blockType){
  // allocate a block on the disk and a buffer in memory to hold the new block of
  // given type using getFreeBlock function and get the return error codes if any.
  int type;
  if(blockType=='R'){
    type=REC;
  }else if(blockType=='I'){
    type=IND_INTERNAL;
  }else if(blockType=='L'){
    type=IND_LEAF;
  }else{
    type=UNUSED_BLK;
  }
  int blockNum=getFreeBlock(type);
  if(!(blockNum >= 0 && blockNum < DISK_BLOCKS)){
    std::cout<<"Error: Block is not available\n";
  }
  // set the blockNum field of the object to that of the allocated block
  // number if the method returned a valid block number,
  // otherwise set the error code returned as the block number.
  this->blockNum=blockNum;
  // (The caller must check if the constructor allocatted block successfully
  // by checking the value of block number field.)
}

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

  memcpy(&head->blockType, bufferPtr, 4);
  memcpy(&head->pblock, bufferPtr + 4, 4);
  memcpy(&head->lblock, bufferPtr + 8, 4);
  memcpy(&head->rblock, bufferPtr + 12, 4);
  memcpy(&head->numEntries, bufferPtr + 16, 4);
  memcpy(&head->numAttrs, bufferPtr + 20, 4);
  memcpy(&head->numSlots, bufferPtr + 24, 4);

  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {

  struct HeadInfo head;

  // get the header using this.getHeader() function
  this->getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    std::cout<<"error at getrecord\n";
    return ret;
  }
  
/*
  // read the block at this.blockNum into a buffer
  unsigned char buffer[BLOCK_SIZE];
  
  if(Disk::readBlock(buffer,this->blockNum)!=SUCCESS){return FAILURE;}*/
  

  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
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

  if (bufferNum == E_BLOCKNOTINBUFFER) {
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum == E_OUTOFBOUND || bufferNum==FAILURE) {
      // std::cout<<"error at lbagbuff\n";
      return bufferNum;
    }

    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }
  else{
    for(int idx=0;idx<BUFFER_CAPACITY;idx++){
      if(!StaticBuffer::metainfo[idx].free){
        if(idx==bufferNum){
        StaticBuffer::metainfo[idx].timeStamp=0;}
        else{
          StaticBuffer::metainfo[idx].timeStamp++;}
      }

    }

  }

  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];
  // std::cout<<"lbagbuffdone\n";
  return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum){
  unsigned char *bufferPtr;

  /* get the starting address of the buffer containing the block
  using loadBlockAndGetBufferPtr(&bufferPtr). */
  int ret=loadBlockAndGetBufferPtr(&bufferPtr);

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  // return the value returned by the call.
  if(ret!=SUCCESS){
    std::cout<<"error at setrecord\n";
    return ret;
  }

  /* get the header of the block using the getHeader() function */

    // get number of attributes in the block.

    // get the number of slots in the block.

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.


	struct HeadInfo head;
	this->getHeader(&head);
	
	int attrs=head.numAttrs;
	int slots=head.numSlots;

  if(slotNum>=slots){
    return E_OUTOFBOUND;
  }

	
	// unsigned char buffer[BLOCK_SIZE];
	// Disk::readBlock(buffer,this->blockNum);

   /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */

	
	int recordSize=attrs*ATTR_SIZE;
	unsigned char *slotPointer=bufferPtr+HEADER_SIZE+slots+(recordSize*slotNum);

  // update dirty bit using setDirtyBit()

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */

    // return SUCCESS
	
	memcpy(slotPointer,rec,recordSize);

  StaticBuffer::setDirtyBit(this->blockNum);
	
	//Disk::writeBlock(bufferPtr,this->blockNum);
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
  RecBuffer recordBlock(this->blockNum);
  recordBlock.getHeader(&head);
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
    if(diff>0){return 1;}
    else if(diff==0){return 0;}
    else{return -1;}
    
}

int BlockBuffer::setHeader(struct HeadInfo *head){

  unsigned char *bufferPtr;
  // get the starting address of the buffer containing the block using
  // loadBlockAndGetBufferPtr(&bufferPtr).

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
      // return the value returned by the call.

  // cast bufferPtr to type HeadInfo*
  int buffstart=loadBlockAndGetBufferPtr(&bufferPtr);
  if(buffstart!=SUCCESS){
    std::cout<<"error at setheader\n";
    return buffstart;
  }
  HeadInfo *bufferHeader = (HeadInfo *)bufferPtr;

  // copy the fields of the HeadInfo pointed to by head (except reserved) to
  // the header of the block (pointed to by bufferHeader)
  //(hint: bufferHeader->numSlots = head->numSlots )

  // update dirty bit by calling StaticBuffer::setDirtyBit()
  // if setDirtyBit() failed, return the error code

  // return SUCCESS;

  bufferHeader->blockType = head->blockType;
  bufferHeader->pblock = head->pblock;
  bufferHeader->lblock = head->lblock;
  bufferHeader->rblock = head->rblock;
  bufferHeader->numAttrs = head->numAttrs;
  bufferHeader->numEntries = head->numEntries;
  bufferHeader->numSlots = head->numSlots;

  int setDirty=StaticBuffer::setDirtyBit(this->blockNum);
  if(setDirty!=SUCCESS)return setDirty;
  return SUCCESS;
}


int BlockBuffer::setBlockType(int blockType){

  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
      // return the value returned by the call.

  int buffstart=loadBlockAndGetBufferPtr(&bufferPtr);
  if(buffstart!=SUCCESS){
    std::cout<<"error at setblocktype\n";
    return buffstart;
  }
  // store the input block type in the first 4 bytes of the buffer.
  // (hint: cast bufferPtr to int32_t* and then assign it)
  *((int32_t *)bufferPtr) = blockType;

  // update the StaticBuffer::blockAllocMap entry corresponding to the
  // object's block number to `blockType`.
  StaticBuffer::blockAllocMap[this->blockNum]=blockType;
  int ret = StaticBuffer::setDirtyBit(this->blockNum);

  if (ret != SUCCESS)
  {
    std::cout << "setDirty failed\n";
    return ret;
  }

  return SUCCESS;
  // update dirty bit by calling StaticBuffer::setDirtyBit()
  // if setDirtyBit() failed
      // return the returned value from the call

  // return SUCCESS
}


int BlockBuffer::getFreeBlock(int blockType){

  // iterate through the StaticBuffer::blockAllocMap and find the block number
  // of a free block in the disk.
  int freeblk=-1;
  for(int i=0;i<DISK_BLOCKS;i++){
    if(StaticBuffer::blockAllocMap[i]==UNUSED_BLK){
      freeblk=i;
      break;
    }
  }

  // if no block is free, return E_DISKFULL.
  if(freeblk==-1){
    return E_DISKFULL;
  }

  // set the object's blockNum to the block number of the free block.
  this->blockNum=freeblk;
  // find a free buffer using StaticBuffer::getFreeBuffer() .
  int bufferNum=StaticBuffer::getFreeBuffer(this->blockNum);
  // if(bufferNum<0 || bufferNum>=BUFFER_CAPACITY){
  //   printf("Error: buffer is full\n");
  //   return bufferNum;
  // }
  // initialize the header of the block passing a struct HeadInfo with values
  // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
  // to the setHeader() function.
  HeadInfo header;
  header.lblock=header.pblock=header.rblock=-1;
  header.numAttrs=header.numEntries=header.numSlots=0;

  // update the block type of the block to the input block type using setBlockType().
  // return block number of the free block.
  int ret = setHeader(&header);

  // std::cout<<"4--\n";

  if (ret != SUCCESS){
    std::cout<<"error at getfreeblock1\n";

    return ret;
  }
  ret = setBlockType(blockType);

  if (ret != SUCCESS){
    std::cout<<"error at getfreeblock2\n";
    return ret;
  }


  return freeblk;
}

int RecBuffer::setSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block using
     loadBlockAndGetBufferPtr(&bufferPtr). */
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }
  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  // return the value returned by the call.

  // get the header of the block using the getHeader() function
  HeadInfo header;
  getHeader(&header);
  int numSlots = header.numSlots; /* the number of slots in the block */
  unsigned char *offset = bufferPtr + HEADER_SIZE;
  memcpy(offset, slotMap, numSlots);
  // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
  // argument `slotMap` to the buffinter replacing the existing slotmap.
  // Note that size of slotmap is `numSlots`
  ret = StaticBuffer::setDirtyBit(this->blockNum);
  // update dirty bit using StaticBuffer::setDirtyBit
  // if setDirtyBit failed, return the value returned by the call
  if(ret!=SUCCESS){
    return ret;
  }

  return SUCCESS;
}

int BlockBuffer::getBlockNum() {
  return this->blockNum;
}

void BlockBuffer::releaseBlock(){

  if (this->blockNum == -1)
  return;

  // else

  /* get the buffer number of the buffer assigned to the block
      using StaticBuffer::getBufferNum().
      (this function return E_BLOCKNOTINBUFFER if the block is not
      currently loaded in the buffer)
  */

  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    if (bufferNum == E_BLOCKNOTINBUFFER)
      return;

    // if the block is present in the buffer, free the buffer
    // by setting the free flag of its StaticBuffer::tableMetaInfo entry
    // to true.
    StaticBuffer::metainfo[bufferNum].free = true;

    // free the block in disk by setting the data type of the entry
    // corresponding to the block number in StaticBuffer::blockAllocMap
    // to UNUSED_BLK.
    StaticBuffer::blockAllocMap[this->blockNum] = UNUSED_BLK;

    // set the object's blockNum to INVALID_BLOCK (-1)
    this->blockNum = -1;
}

int IndInternal::getEntry(void *ptr, int indexNum) {
  // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
  //     return E_OUTOFBOUND.
  if(indexNum<0 || indexNum>=MAX_KEYS_INTERNAL){return E_OUTOFBOUND;}

  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  //     return the value returned by the call.
  int ret=loadBlockAndGetBufferPtr(&bufferPtr);
  if(ret!=SUCCESS){return ret;}

  // typecast the void pointer to an internal entry pointer
  struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;

  /*
  - copy the entries from the indexNum`th entry to *internalEntry
  - make sure that each field is copied individually as in the following code
  - the lChild and rChild fields of InternalEntry are of type int32_t
  - int32_t is a type of int that is guaranteed to be 4 bytes across every
    C++ implementation. sizeof(int32_t) = 4
  */

  /* the indexNum'th entry will begin at an offset of
     HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
     from bufferPtr */
  unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);

  memcpy(&(internalEntry->lChild), entryPtr, sizeof(int32_t));
  memcpy(&(internalEntry->attrVal), entryPtr + 4, sizeof(Attribute));
  memcpy(&(internalEntry->rChild), entryPtr + 20, 4);

  return SUCCESS;
  
}

int IndLeaf::getEntry(void *ptr, int indexNum) {

  // if the indexNum is not in the valid range of [0, MAX_KEYS_LEAF-1]
  //     return E_OUTOFBOUND.

  if(indexNum<0 || indexNum>=MAX_KEYS_LEAF){return E_OUTOFBOUND;}
  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  //     return the value returned by the call.
  int ret =loadBlockAndGetBufferPtr(&bufferPtr);
  if(ret!=SUCCESS){return ret;}

  // copy the indexNum'th Index entry in buffer to memory ptr using memcpy

  /* the indexNum'th entry will begin at an offset of
     HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
  unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
  memcpy((struct Index *)ptr, entryPtr, LEAF_ENTRY_SIZE);

  return SUCCESS;
}

int IndLeaf::setEntry(void *ptr, int indexNum) {

  // if the indexNum is not in the valid range of [0, MAX_KEYS_LEAF-1]
  //     return E_OUTOFBOUND.
  if(indexNum<0 || indexNum>=MAX_KEYS_LEAF)return E_OUTOFBOUND;

  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  //     return the value returned by the call.
  int ret=loadBlockAndGetBufferPtr(&bufferPtr);
  if(ret!=SUCCESS){return ret;}

  // copy the Index at ptr to indexNum'th entry in the buffer using memcpy

  /* the indexNum'th entry will begin at an offset of
     HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
  unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
  memcpy(entryPtr, (struct Index *)ptr, LEAF_ENTRY_SIZE);

  // update dirty bit using setDirtyBit()
  // if setDirtyBit failed, return the value returned by the call
  ret=StaticBuffer::setDirtyBit(this->blockNum);
  if(ret!=SUCCESS){
    return ret;
  }
  return SUCCESS;
}

int IndInternal::setEntry(void *ptr, int indexNum) {
  // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
  //     return E_OUTOFBOUND.
  if(indexNum<0 || indexNum>=MAX_KEYS_INTERNAL)return E_OUTOFBOUND;


  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  //     return the value returned by the call.
  int ret=loadBlockAndGetBufferPtr(&bufferPtr);
  if(ret!=SUCCESS)return ret;

  // typecast the void pointer to an internal entry pointer
  struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;

  /*
  - copy the entries from *internalEntry to the indexNum`th entry
  - make sure that each field is copied individually as in the following code
  - the lChild and rChild fields of InternalEntry are of type int32_t
  - int32_t is a type of int that is guaranteed to be 4 bytes across every
    C++ implementation. sizeof(int32_t) = 4
  */

  /* the indexNum'th entry will begin at an offset of
     HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
     from bufferPtr */

  unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);

  memcpy(entryPtr, &(internalEntry->lChild), 4);
  memcpy(entryPtr + 4, &(internalEntry->attrVal), ATTR_SIZE);
  memcpy(entryPtr + 20, &(internalEntry->rChild), 4);


  // update dirty bit using setDirtyBit()
  // if setDirtyBit failed, return the value returned by the call

  ret=StaticBuffer::setDirtyBit(this->blockNum);
  if(ret!=SUCCESS){
    return ret;
  }
  return SUCCESS;
}
