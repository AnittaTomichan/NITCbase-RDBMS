#include "StaticBuffer.h"
#include <iostream>
using std::cout;
unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];

StaticBuffer::StaticBuffer(){
  // copy blockAllocMap blocks from disk to buffer (using readblock() of disk)
  // blocks 0 to 3
    unsigned char buffer[BLOCK_SIZE];
	int blockAllocMapSlot=0;
    for(int i=0;i<4;i++){
		Disk::readBlock(buffer,i);
		for(int slot=0;slot<BLOCK_SIZE;slot++){
			StaticBuffer::blockAllocMap[blockAllocMapSlot++]=buffer[slot];
		}
	}

	for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++){
		metainfo[bufferIndex].free=true;
		metainfo[bufferIndex].dirty=false;
		metainfo[bufferIndex].timeStamp=-1;
		metainfo[bufferIndex].blockNum=-1;
	}
}

StaticBuffer::~StaticBuffer(){
	 // copy blockAllocMap blocks from buffer to disk(using writeblock() of disk)
	int blockMapSlot=0;
	unsigned char buffer[BLOCK_SIZE];
	 for(int i=0;i<4;i++){
		for(int slot=0;slot<BLOCK_SIZE;slot++){
			buffer[slot]=blockAllocMap[blockMapSlot++];
		}
		Disk::writeBlock(buffer,i);
	}

	for(int idx=0;idx<BUFFER_CAPACITY;idx++){
		if(metainfo[idx].free==false && metainfo[idx].dirty==true){
			Disk::writeBlock(blocks[idx],metainfo[idx].blockNum);
		}
	}
}

// int StaticBuffer::getFreeBuffer(int blockNum){
// 	if(blockNum < 0 || blockNum>=DISK_BLOCKS){
// 		return E_OUTOFBOUND;
// 	}
// 	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++){
// 		if(metainfo[bufferIndex].free==false){
// 			metainfo[bufferIndex].timeStamp++;
// 		}
// 	}

// 	int lruBufferIdx,largestTimeStamp=0;

// 	int bufferNum=-1;

// 	for(int idx=0;idx<BUFFER_CAPACITY;idx++){
// 		if(metainfo[idx].timeStamp>largestTimeStamp){
// 			largestTimeStamp=metainfo[idx].timeStamp;
// 			lruBufferIdx=idx;
// 		}
// 		if(metainfo[idx].free){
// 			bufferNum=idx;
// 			break;

// 		}
// 	}


// 	if(bufferNum==-1){
// 		if(metainfo[lruBufferIdx].dirty==true){
// 			Disk::writeBlock(blocks[lruBufferIdx],metainfo[lruBufferIdx].blockNum);
// 			bufferNum=lruBufferIdx;

// 		}
// 	}
	
// 	metainfo[bufferNum].free=false;
// 	metainfo[bufferNum].blockNum=blockNum;
// 	metainfo[bufferNum].dirty=false;
// 	metainfo[bufferNum].timeStamp=0;
// 	return bufferNum;
// }
int StaticBuffer::getFreeBuffer(int blockNum)
{
    // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.
    if (blockNum < 0 || blockNum >= DISK_BLOCKS)
    {
        return E_OUTOFBOUND;
    }

    // increase the timeStamp in metaInfo of all occupied buffers.
    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++)
    {
        if (!metainfo[bufferIndex].free)
        {
            metainfo[bufferIndex].timeStamp++;
        }
    }

    // let bufferNum be used to store the buffer number of the free/freed buffer.
    int bufferNum;
    bool bufferFound = false;
    // iterate through metainfo and check if there is any buffer free
    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++)
    {
        if (metainfo[bufferIndex].free)
        {
            bufferNum = bufferIndex;
            bufferFound = true;
            break;
        }
    }

    // if a free buffer is available, set bufferNum = index of that free buffer.

    // if a free buffer is not available,
    //     find the buffer with the largest timestamp
    //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    //     set bufferNum = index of this buffer

    if (!bufferFound)
    {
        int maxTimeStamp = -1;
        for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++)
        {
            if (metainfo[bufferIndex].timeStamp > maxTimeStamp)
            {
                maxTimeStamp = metainfo[bufferIndex].timeStamp;
                bufferNum = bufferIndex;
            }
        }
        if (metainfo[bufferNum].dirty)
        {
            Disk::writeBlock(blocks[bufferNum], metainfo[bufferNum].blockNum);
        }
    }

    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.
    metainfo[bufferNum].free = false;
    metainfo[bufferNum].dirty = false;
    metainfo[bufferNum].blockNum = blockNum;
    metainfo[bufferNum].timeStamp = 0; // or -1

    return bufferNum;

    // return the bufferNum.
}

int StaticBuffer::getBufferNum(int blockNum){
	if(blockNum<0 || blockNum>DISK_BLOCKS){
		return E_OUTOFBOUND;
	}
	
	for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++){
		if(metainfo[bufferIndex].blockNum==blockNum){
			return bufferIndex;
		}
	}
	return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){
	int bufferIndex=getBufferNum(blockNum);

	if(bufferIndex==E_BLOCKNOTINBUFFER){return E_BLOCKNOTINBUFFER;}
	if(bufferIndex==E_OUTOFBOUND){return E_OUTOFBOUND;}
	metainfo[bufferIndex].dirty=true;

	return SUCCESS;
}

int StaticBuffer::getStaticBlockType(int blockNum){
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.

    if(blockNum<0 or blockNum>=DISK_BLOCKS){
        return E_OUTOFBOUND;
    }
    // Access the entry in block allocation map corresponding to the blockNum argument
    // and return the block type after type casting to integer.
    return (int)blockAllocMap[blockNum];
}
