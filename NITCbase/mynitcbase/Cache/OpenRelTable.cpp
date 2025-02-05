#include "OpenRelTable.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];


AttrCacheEntry *createAttrCacheEntryList(int numberOfAttr)
{
    AttrCacheEntry *head = nullptr, *curr = nullptr;
    head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    numberOfAttr--;
    while (numberOfAttr--)
    {
        curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        curr = curr->next;
    }
    curr->next = nullptr;
    return head;
}

OpenRelTable::OpenRelTable(){
	for(int i=0;i<MAX_OPEN;i++){
		RelCacheTable::relCache[i]=nullptr;
		AttrCacheTable::attrCache[i]=nullptr;
		tableMetaInfo[i].free=true;
	}
	/************ Setting up Relation Cache entries ************/
	  // (we need to populate relation cache with entries for the relation catalog
	  //  and attribute catalog.)

	  /**** setting up Relation Catalog relation and Attribute Catalog relation  in the Relation Cache Table****/
	RecBuffer relCatBlock(RELCAT_BLOCK);
	Attribute relCatRecord[RELCAT_NO_ATTRS];
	RelCacheEntry *relCacheEntry=nullptr;
		
	for(int relId=RELCAT_RELID;relId<=ATTRCAT_RELID+1;relId++){
		
		relCatBlock.getRecord(relCatRecord, relId);
		relCacheEntry = (RelCacheEntry *) malloc (sizeof(RelCacheEntry));
		RelCacheTable::recordToRelCatEntry(relCatRecord, &(relCacheEntry->relCatEntry));
		relCacheEntry->recId.block=RELCAT_BLOCK;
		relCacheEntry->recId.slot=relId;
		relCacheEntry->searchIndex={-1,-1};
		RelCacheTable::relCache[relId]=relCacheEntry;
	}
	
	/************ Setting up Attribute cache entries ************/
	  // (we need to populate attribute cache with entries for the relation catalog
	  //  and attribute catalog.)

	  /**** setting up Relation Catalog relation and Atribute Catalog Relation in the Attribute Cache Table ****/
	 RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;
    for (int relId = RELCAT_RELID, recordId = 0; relId <= ATTRCAT_RELID+1; relId++)
    {
        int numberOfAttr = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
        head = createAttrCacheEntryList(numberOfAttr);
        attrCacheEntry = head;
        while (numberOfAttr--)
        {
            attrCatBlock.getRecord(attrCatRecord, recordId);
            AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &(attrCacheEntry->attrCatEntry));
            attrCacheEntry->recId.block = ATTRCAT_BLOCK;
            attrCacheEntry->recId.slot = recordId++;

            attrCacheEntry = attrCacheEntry->next;
        }
        AttrCacheTable::attrCache[relId] = head;
    }
		
	
 /************ Setting up tableMetaInfo entries ************/

  // in the tableMetaInfo array
  //   set free = false for RELCAT_RELID and ATTRCAT_RELID
  //   set relname for RELCAT_RELID and ATTRCAT_RELID
  tableMetaInfo[RELCAT_RELID].free=false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName,RELCAT_RELNAME);
  tableMetaInfo[ATTRCAT_RELID].free=false;
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName,ATTRCAT_RELNAME);
}


int OpenRelTable::getRelId( char relName[ATTR_SIZE]) {

 /* if(strcmp(relName,RELCAT_RELNAME)==0){
  	return RELCAT_RELID;
  }
  if(strcmp(relName,ATTRCAT_RELNAME)==0){
  	return ATTRCAT_RELID;
  }
  if(strcmp(relName,"Students")==0){  //stage4 exercise1
  return 2;
  }*/
  
    for(int relId=0;relId<MAX_OPEN;relId++){
     if(strcmp(tableMetaInfo[relId].relName,relName)==0 && tableMetaInfo[relId].free==false)
       return relId ;
    }
     
  // if found return the relation id, else indicate that the relation do not
  // have an entry in the Open Relation Table.

  return E_RELNOTOPEN;
}



int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
    for(int relId=0;relId<MAX_OPEN;relId++){
      if(tableMetaInfo[relId].free)
       return relId;
      }

  return E_CACHEFULL;
}

int OpenRelTable::openRel( char relName[ATTR_SIZE])
{
    int relId = OpenRelTable::getRelId(relName);

    if (relId >= 0 && relId < MAX_OPEN)
    {
        // (checked using OpenRelTable::getRelId())

        // return that relation id;
        return relId;
    }

    /* find a free slot in the Open Relation Table
       using OpenRelTable::getFreeOpenRelTableEntry(). */
    relId = OpenRelTable::getFreeOpenRelTableEntry();
    if (relId == E_CACHEFULL)
    {
        return E_CACHEFULL;
    }


  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/
      Attribute attrVal;
      strcpy(attrVal.sVal,relName);
      RelCacheTable::resetSearchIndex(RELCAT_RELID);
      

  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId=BlockAccess::linearSearch(RELCAT_RELID,RELCAT_ATTR_RELNAME,attrVal,EQ);

  if ( relcatRecId.block ==-1 && relcatRecId.slot== -1) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      
      
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
  RecBuffer relationBuffer(relcatRecId.block);
    Attribute relationRecord[RELCAT_NO_ATTRS];
    relationBuffer.getRecord(relationRecord, relcatRecId.slot);

    RelCacheEntry *relCacheBuffer = nullptr;
    relCacheBuffer = (RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    RelCacheTable::recordToRelCatEntry(relationRecord, &(relCacheBuffer->relCatEntry));
    relCacheBuffer->recId.block = relcatRecId.block;
    relCacheBuffer->recId.slot = relcatRecId.slot;
    RelCacheTable::relCache[relId] = relCacheBuffer;
    /****** Setting up Attribute Cache entry for the relation ******/

    // let listHead be used to hold the head of the linked list of attrCache entries.
    AttrCacheEntry *listHead = nullptr, *attrCacheEntry = nullptr;
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    /*iterate over all the entries in the Attribute Catalog corresponding to each
    attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
    care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
    corresponding to Attribute Catalog before the first call to linearSearch().*/
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    int numOfAttr = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
    listHead = createAttrCacheEntryList(numOfAttr);
     attrCacheEntry = listHead;
    for (int attr = 0; attr < numOfAttr; attr++)
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId=BlockAccess::linearSearch(ATTRCAT_RELID,RELCAT_ATTR_RELNAME,attrVal,EQ);

      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
       RecBuffer attrCatBlock(attrcatRecId.block);
        attrCatBlock.getRecord(attrCatRecord, attrcatRecId.slot);
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &(attrCacheEntry->attrCatEntry));
        attrCacheEntry->recId.block = attrcatRecId.block;
        attrCacheEntry->recId.slot = attrcatRecId.slot;
        attrCacheEntry = attrCacheEntry->next;
  }
  AttrCacheTable::attrCache[relId]=listHead;

  // set the relIdth entry of the AttrCacheTable to listHead.

  /****** Setting up metadata in the Open Relation Table for the relation******/
  tableMetaInfo[relId].free=false;
  strcpy(tableMetaInfo[relId].relName,relName);

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.

  return relId;
}


int OpenRelTable::closeRel(int relId) {
  if (relId==RELCAT_RELID || relId==ATTRCAT_RELID) {
    return E_NOTPERMITTED;
  }

  if (  relId < 0 ||  relId>= MAX_OPEN ) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free) {
    return E_RELNOTOPEN;
  }
  
  if (AttrCacheTable::attrCache[relId] == nullptr)
    {
        return E_RELNOTOPEN;
    }

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  
  free(RelCacheTable::relCache[relId]=nullptr);
  AttrCacheEntry *head=AttrCacheTable::attrCache[relId], *next=head->next;
   while (next)
    {
        free(head);
        head = next;
        next = next->next;
    }
    free(head);
    tableMetaInfo[relId].free = true;
    // update `tableMetaInfo` to set `relId` as a free slot
    // update `relCache` and `attrCache` to set the entry at `relId` to nullptr
    AttrCacheTable::attrCache[relId] = nullptr;
    RelCacheTable::relCache[relId] = nullptr;

  return SUCCESS;
}

OpenRelTable::~OpenRelTable() {

  // close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }

  // free the memory allocated for rel-id 0 and 1 in the caches
}


