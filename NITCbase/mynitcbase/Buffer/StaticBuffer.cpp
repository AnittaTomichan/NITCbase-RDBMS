#include "StaticBuffer.h"
#include <iostream>
using std::cout;
unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];

StaticBuffer::StaticBuffer(){
  // copy blockAllocMap blocks from disk to buffer (using readblock() of disk)
  // blocks 0 to 3
    for(int i=0,blockMapSlot=0;i<4;i++){
		unsigned char buffer[BLOCK_SIZE];
		Disk::readBlock(buffer,i);
		for(int slot=0;slot<BLOCK_SIZE;slot++,blockMapSlot++){
			StaticBuffer::blockAllocMap[blockMapSlot]=buffer[slot];
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
	for(int i=0,blockMapSlot=0;i<4;i++){
		unsigned char buffer[BLOCK_SIZE];
		for(int slot=0;slot<BLOCK_SIZE;slot++,blockMapSlot++){
			buffer[slot]=blockAllocMap[blockMapSlot];
		}
		Disk::writeBlock(buffer,i);
	}

	for(int idx=0;idx<BUFFER_CAPACITY;idx++){
		if(metainfo[idx].free==false && metainfo[idx].dirty==true){
			Disk::writeBlock(blocks[idx],metainfo[idx].blockNum);
		}
	}
}

int StaticBuffer::getFreeBuffer(int blockNum){
	if(blockNum < 0 || blockNum>=DISK_BLOCKS){
		return E_OUTOFBOUND;
	}
	for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++){
		if(metainfo[bufferIndex].free==false){
			metainfo[bufferIndex].timeStamp++;
		}
	}

	int lruBufferIdx,largestTimeStamp=0;

	int bufferNum=-1;

	for(int idx=0;idx<BUFFER_CAPACITY;idx++){
		if(metainfo[idx].timeStamp>largestTimeStamp){
			largestTimeStamp=metainfo[idx].timeStamp;
			lruBufferIdx=idx;
		}
		if(metainfo[idx].free){
			bufferNum=idx;
			break;

		}
	}


	if(bufferNum==-1){
		if(metainfo[lruBufferIdx].dirty==true){
			Disk::writeBlock(blocks[lruBufferIdx],metainfo[lruBufferIdx].blockNum);
			bufferNum=lruBufferIdx;

		}
	}
	
	metainfo[bufferNum].free=false;
	metainfo[bufferNum].blockNum=blockNum;
	metainfo[bufferNum].dirty=false;
	metainfo[bufferNum].timeStamp=0;
	return bufferNum;
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
	else{metainfo[bufferIndex].dirty=true;}

	return SUCCESS;
}
