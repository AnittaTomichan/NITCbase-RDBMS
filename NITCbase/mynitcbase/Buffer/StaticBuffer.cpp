#include "StaticBuffer.h"
#include <iostream>
using std::cout;


unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {

  // initialise all blocks as free
  for ( int bufferIndex = 0 ;bufferIndex< BUFFER_CAPACITY;bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].dirty=false;
    metainfo[bufferIndex].blockNum=-1;
    metainfo[bufferIndex].timeStamp=0;
  }
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/
StaticBuffer::~StaticBuffer() {}

int StaticBuffer::getFreeBuffer(int blockNum) {
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }
  int allocatedBuffer;

  // iterate through all the blocks in the StaticBuffer
  // find the first free block in the buffer (check metainfo)
  // assign allocatedBuffer = index of the free block
  for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++){
   if(metainfo[bufferIndex].free){
      allocatedBuffer=bufferIndex;
      break;
  }
 }
   metainfo[allocatedBuffer].free=false;
   metainfo[allocatedBuffer].blockNum=blockNum;
 return allocatedBuffer;
 }

/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

 for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
    if (!metainfo[bufferIndex].free && metainfo[bufferIndex].blockNum == blockNum)  {
      return bufferIndex;  // Found the block in this buffer index
    }
  }

  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}
