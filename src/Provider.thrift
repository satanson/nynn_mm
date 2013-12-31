namespace cpp nynn.mm

service Provider{
	bool createSubgraph(1:i32 subgraphKey);
	bool destroySubgraph(1:i32 subgraphKey);		
	bool attachSubgraph(1:i32 subgraphKey);
	bool detachSubgraph(1:i32 subgraphKey)		
	list<i32>  getSubgraphKeys();
	i32  getWidthOfVertexInterval();
	
	bool lock(1:i32 vtxno,2:i32 flag);
	bool unlock(1:i32 vtxno);
	
	i32 getSize(1:i32 vtxno);
	i32 getHeadBlkno(1:i32 vtxno);
	i32 getTailBlkno(1:i32 vtxno);
	list<i32> readAllBlknos(1:i32 vtxno);
	list<byte> read(1:i32 vtxno,2:i32 blkno);
	list<byte> readn(1:i32 vtxno,2:i32 blkno,3:i32 n);
	
	i32 insertPrev(1:i32 vtxno, 2:i32 nextBlkno, 3:list<byte> xblk);	
	i32 insertNext(1:i32 vtxno, 2:i32 prevBlkno, 3:list<byte> xblk);	
	bool remove(1:i32 vtxno,2:i32 blkno);

	i32 unshift(1:i32 vtxno, 2:list<byte> newXHeadBlk);
	bool shift(1:i32 vtxno);
	i32 push(1:i32 vtxno,2:list<byte> newXTailBlk);
	bool pop(1:i32 vtxno)
}
