
int tst_bit(char *buf, int bit)
{
  // in Chapter 11.8.1
  return buf[bit/8] & (1 << (bit%8));
}

int set_bit(char *buf, int bit)
{
  // in Chapter 11.8.1
  buf[bit/8] |= (1 << (bit % 8));
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
        put_block(dev, imap, buf);
        printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}

int balloc(int dev) // returns a FREE disk block number
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < nblocks; i++){
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
        put_block(dev, imap, buf);
        printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}

int clr_bit(char *buf, int bit)  // clear bit in char buf[BLKSIZE]
{
	return buf[bit/8] &= ~(1 << (bit%8));
}

int incFreeInodes(int dev)
{
	char buf[BLKSIZE];
	// inc free inodes count in SUPER and GD
	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_inodes_count++;
	put_block(dev, 1, buf);
	gp = (GD *)buf;
	gp->bg_free_inodes_count++;
	put_block(dev, 2, buf);
}

int idalloc(int dev, int ino)
{
	int i;
	char buf[BLKSIZE];
	//MTABLE *mp = (MTABLE *)get_mtable(dev);
	if(ino > ninodes)  // niodes global
	{
		printf("inumber %d out of range\n", ino);
		return -1;
	}
	// get inode bitmap block
	get_block(dev, imap, buf);
	clr_bit(buf, ino-1);
	// write buf back
	put_block(dev, imap, buf);
	// update free inode count in SUPER and GD
	incFreeInodes(dev);
}

int bdalloc(int dev, int ib)
{
	int i;
	char buf[BLKSIZE];
	//MTABLE *mp = (MTABLE *)get_mtable(dev);
	if(ib > nblocks)  // nblocks global
	{
		printf("inumber %d out of range\n", ib);
		return -1;
	}
	// get block bitmap block
	get_block(dev, bmap, buf);
	clr_bit(buf, ib-1);
	// write buf back
	put_block(dev, bmap, buf);
	// update free inode count in SUPER and GD
	incFreeInodes(dev);
}
