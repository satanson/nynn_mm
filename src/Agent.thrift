namespace cpp nynn.mm

service Agent{
	bool lock(1:i32 vtxno);
	bool unlock(1:i32 vtxno);
	i32  getHeadBlkno(1:i32 vtxno);
	i32  getTailBlkno(1:i32 vtxno);
	list<byte> read(1:i32 vtxno,2:i32 blkno);
}
