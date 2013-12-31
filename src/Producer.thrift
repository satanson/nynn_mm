namespace cpp nynn.mm

service Producer{
	bool report(1:list<i32> subgraphKeys,2:string hostname);
	string getHost(1:i32 subgraphKey);

	i32 insertPrev(1:i32 vtxno,2:i32 nextBlkno,3:list<byte> xblk);
	i32 insertNext(1:i32 vtxno,2:i32 prevBlkno,3:list<byte> xblk);
	bool remove(1:i32 vtxno, 2:i32 blkno);
	i32 unshift(1:i32 vtxno,2:list<byte> newXHeadBlk);
	bool shift(1:i32 vtxno);
	i32 push(1:i32 vtxno,2:list<byte> newXTailBlk);
	bool pop(1:i32 vtxno)
}
