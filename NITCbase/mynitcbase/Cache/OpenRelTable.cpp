#include "OpenRelTable.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];


/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/

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
	
	
	for(int relId=RELCAT_RELID,recordId=0;relId<=ATTRCAT_RELID+1;relId++){
		int attrNums=RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
		AttrCacheEntry *attrCacheEntry=nullptr, *head=nullptr, *curr=nullptr;
		
		head=curr=(AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
		int size=attrNums-1;
		while(size--){
			curr->next=(AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
			curr=curr->next;
		}
		curr->next=nullptr;
		
		attrCacheEntry=head;
		
		while(attrNums--){
			attrCatBlock.getRecord(attrCatRecord, recordId);
			
			AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&(attrCacheEntry->attrCatEntry));
			attrCacheEntry->recId.slot=recordId++;
			attrCacheEntry->recId.block=ATTRCAT_BLOCK;
			attrCacheEntry=attrCacheEntry->next;
		}
		AttrCacheTable::attrCache[relId]=head;
		
	
	}
	
	/************ Setting up tableMetaInfo entries ************/

	for(int i=RELCAT_RELID;i<=ATTRCAT_RELID;i++){
		tableMetaInfo[i].free=false;
		if(i==RELCAT_RELID){
			strcpy(tableMetaInfo[i].relName,RELCAT_RELNAME);
		}
		else{
			strcpy(tableMetaInfo[i].relName,ATTRCAT_RELNAME);
		}
	}
	
	
}

/*
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  // if relname is RELCAT_RELNAME, return RELCAT_RELID
  // if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID
  if(strcmp(relName,RELCAT_RELNAME)==0){
  	return RELCAT_RELID;
  }
  if(strcmp(relName,ATTRCAT_RELNAME)==0){
  	return ATTRCAT_RELID;
  }
  if (strcmp(relName, "Students") == 0){return 2;}


  return E_RELNOTOPEN;
}
*/

OpenRelTable::~OpenRelTable() {

  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i);
    }
  }

  /**** Closing the catalog relations in the relation cache ****/

  //releasing the relation cache entry of the attribute catalog

  if (RelCacheTable::relCache[ATTRCAT_RELID]->dirty==true) {

      /* Get the Relation Catalog entry from RelCacheTable::relCache
      Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
      RelCatEntry relCatBuffer;
      RelCacheTable::getRelCatEntry(ATTRCAT_RELID,&relCatBuffer);
      Attribute relCatRecord[RELCAT_NO_ATTRS];
      RelCacheTable::relCatEntryToRecord(&relCatBuffer,relCatRecord);
      RecId recId=RelCacheTable::relCache[ATTRCAT_RELID]->recId;
      // declaring an object of RecBuffer class to write back to the buffer
      RecBuffer relCatBlock(recId.block);

      // Write back to the buffer using relCatBlock.setRecord() with recId.slot
      relCatBlock.setRecord(relCatRecord,recId.slot);
  }
  // free the memory dynamically allocated to this RelCacheEntry
  free(RelCacheTable::relCache[ATTRCAT_RELID]);


  //releasing the relation cache entry of the relation catalog

  if(RelCacheTable::relCache[RELCAT_RELID]->dirty==true) {

      /* Get the Relation Catalog entry from RelCacheTable::relCache
      Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
      RelCatEntry relCatBuffer;
      RelCacheTable::getRelCatEntry(RELCAT_RELID,&relCatBuffer);
      Attribute relCatRecord[ATTRCAT_NO_ATTRS];
      RelCacheTable::relCatEntryToRecord(&relCatBuffer,relCatRecord);
      RecId recId=RelCacheTable::relCache[RELCAT_RELID]->recId;

      // declaring an object of RecBuffer class to write back to the buffer
      RecBuffer attrCatRecord(recId.block);

      // Write back to the buffer using relCatBlock.setRecord() with recId.slot
      attrCatRecord.setRecord(relCatRecord,recId.slot);
  }
  // free the memory dynamically allocated for this RelCacheEntry
  free(RelCacheTable::relCache[RELCAT_RELID]);


  // free the memory allocated for the attribute cache entries of the
  // relation catalog and the attribute catalog
  for(int relId=RELCAT_RELID;relId<=ATTRCAT_RELID;relId++){
      AttrCacheEntry *curr=AttrCacheTable::attrCache[relId],*next=NULL;
      while(curr!=nullptr){
          next=curr->next;
          if(curr->dirty==true){
              AttrCatEntry attrCatEntry=curr->attrCatEntry;
              Attribute attrCatrecord[ATTRCAT_NO_ATTRS];
              AttrCacheTable::attrCatEntryToRecord(&attrCatEntry,attrCatrecord);
              RecBuffer attrCatBlock(curr->recId.block);
              attrCatBlock.setRecord(attrCatrecord,curr->recId.slot);
          }
          free(curr);
          curr=next;
      }
  }
}

int OpenRelTable::getFreeOpenRelTableEntry(){
	for(int i=2;i<MAX_OPEN;i++){
		if(tableMetaInfo[i].free){
			return i;
		}
	}
	return E_CACHEFULL;
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE]){
	for(int i=0;i<MAX_OPEN;i++){
		if(tableMetaInfo[i].free==false && strcmp(tableMetaInfo[i].relName,relName)==0){
			return i;
		}
	}
	return E_RELNOTOPEN;
}

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

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  int relId= OpenRelTable::getRelId(relName);
  if(relId>=0 && relId<MAX_OPEN){
    return relId; 
  }

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
  int freeSlot=OpenRelTable::getFreeOpenRelTableEntry();

  if (freeSlot==E_CACHEFULL){
    return E_CACHEFULL;
  }

  // let relId be used to store the free slot.
  relId=freeSlot;

  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/

  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  Attribute attrVal;
  strcpy(attrVal.sVal,relName);
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  RecId relcatRecId=BlockAccess::linearSearch(RELCAT_RELID,RELCAT_ATTR_RELNAME,attrVal,EQ);
  
  if(relcatRecId.block==-1 and relcatRecId.slot==-1){
  	return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
  
  RecBuffer relationBuffer(relcatRecId.block);
  Attribute relationRecord[RELCAT_NO_ATTRS];
  RelCacheEntry *relCacheBuffer=nullptr;
  
  relationBuffer.getRecord(relationRecord,relcatRecId.slot);
  
  //allocate memory for relcachentry
  relCacheBuffer=(RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  RelCacheTable::recordToRelCatEntry(relationRecord,&(relCacheBuffer->relCatEntry));
  
  //update recid fields
  relCacheBuffer->recId.block=relcatRecId.block;
  relCacheBuffer->recId.slot=relcatRecId.slot;

  RelCacheTable::relCache[relId]=relCacheBuffer;
  
  /****** Setting up Attribute Cache entry for the relation ******/
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  // let listHead be used to hold the head of the linked list of attrCache entries.
  AttrCacheEntry* head=nullptr, *attrCacheEntry=nullptr;
  
  int attrNums=RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
  head=createAttrCacheEntryList(attrNums);
  attrCacheEntry=head;
  
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  
  for(int attr=0;attr<attrNums;attr++)
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
      attrCatBlock.getRecord(attrCatRecord,attrcatRecId.slot);
      AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&(attrCacheEntry->attrCatEntry));
      
      attrCacheEntry->recId.block=attrcatRecId.block;
      attrCacheEntry->recId.slot=attrcatRecId.slot;
      attrCacheEntry=attrCacheEntry->next;
  }

  // set the relIdth entry of the AttrCacheTable to listHead.
  AttrCacheTable::attrCache[relId]=head;
  
  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.
  tableMetaInfo[relId].free=false;
  strcpy(tableMetaInfo[relId].relName,relName);

  return relId;
}

int OpenRelTable::closeRel(int relId) {
  if (relId==RELCAT_RELID || relId==ATTRCAT_RELID) {
    return E_NOTPERMITTED;
  }

  if (relId<0 || relId>=MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free) {
    return E_RELNOTOPEN;
  }

  if(RelCacheTable::relCache[relId]->dirty==true){
    /* Get the Relation Catalog entry from RelCacheTable::relCache
    Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
    Attribute relCatBuffer[RELCAT_NO_ATTRS];
    RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry),relCatBuffer);

    // declaring an object of RecBuffer class to write back to the buffer
    RecId recId=RelCacheTable::relCache[relId]->recId;
    RecBuffer relCatBlock(recId.block);

    // Write back to the buffer using relCatBlock.setRecord() with recId.slot
    relCatBlock.setRecord(relCatBuffer,RelCacheTable::relCache[relId]->recId.slot);
  }

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  free(RelCacheTable::relCache[relId]);
  AttrCacheEntry *head=AttrCacheTable::attrCache[relId];
  AttrCacheEntry *next=head->next;
  while(true){
    if(head->dirty){
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      AttrCacheTable::attrCatEntryToRecord(&(head->attrCatEntry),attrCatRecord);
      RecBuffer attrCatBuf(head->recId.block);
      attrCatBuf.setRecord(attrCatRecord,head->recId.slot);
    }
  	free(head);
  	head=next;
    if(head==NULL){break;}
  	next=next->next;
  }
  // update `tableMetaInfo` to set `relId` as a free slot
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr
  tableMetaInfo[relId].free=true;
  RelCacheTable::relCache[relId]=nullptr;
  AttrCacheTable::attrCache[relId]=nullptr;

  return SUCCESS;
}



