/************* cd_ls_pwd.c file **************/
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int ls_file(MINODE *mip, char *name)
{
  // READ Chapter 11.7.3 HOW TO ls
  int i;
  char ftime[64], l_name[BLKSIZE], buf[BLKSIZE];
  
  if((mip->INODE.i_mode & 0xF000) == 0x8000)  // if (S_ISREG())
  	printf("%c", '-');
  if((mip->INODE.i_mode & 0xF000) == 0x4000)  // if (S_ISDIR())
  	printf("%c", 'd');
  if((mip->INODE.i_mode & 0xF000) == 0xA000)  // if (S_ISLNK())
  {
  	printf("%c", 'l');
  	/*get_block(mip->dev, mip->INODE.i_block[0], buf);
  	strcpy(l_name, buf);
  	put_block(mip->dev, mip->INODE.i_block[0], buf);
  	l_name[strlen(l_name)] = 0;*/
  }
  for(i = 8; i >= 0; i--)
  {
  	if(mip->INODE.i_mode & (1<<i))        // print r|w|x
  		printf("%c", t1[i]);
  	else if((mip->INODE.i_mode & 0xF000) == 0xA000)
  		printf("%C", t1[i]);
  	else                            // or print -
  		printf("%c", t2[i]);   
  }
  //(p 304)
  printf("%4d ", mip->INODE.i_links_count);     // hard-link count
  printf("%4d ", mip->INODE.i_gid);              // group ID
  printf("%4d ", mip->INODE.i_uid);              // owner uid
  printf("%8d ", mip->INODE.i_size);             // file size in bytes
  
  // print time
  strcpy(ftime, ctime(&mip->INODE.i_mtime));  // pass & of time field prints i_atime in calendar form
  ftime[strlen(ftime)-1] = 0;  // kill \n at end
  printf("%s  ", ftime);
  /*strcpy(ftime, ctime(&ip->i_ctime));    // print time in calendar form
  ftime[strlen(ftime)-1] = 0;            // kill \n at end
  printf("%s ", ftime);*/
  
  // print name
  printf("%s", name);             // print file basename
  
  // print -> linkname if symbolic file
  if((mip->INODE.i_mode & 0xF000) == 0xA000)
  {
  	if(readlink(name, l_name, BLKSIZE)>0)
  		printf(" -> %s", l_name);
  }
  //iput(mip);
  printf("\n");
}

int ls_dir(MINODE *mip)
{
  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  MINODE *fmip;
  
  // Assume DIR has only one data block i_block[0]
  get_block(dev, mip->INODE.i_block[0], buf); 
  dp = (DIR *)buf;  // typecasting
  cp = buf;
  
  while (cp < buf + BLKSIZE)
  {
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
     //printf("[%d %s]  ", dp->inode, temp); // print [inode# name]
     
     fmip = iget(dev, dp->inode);
     //fmip->dirty = 0;
     if(fmip)
     {
       ls_file(fmip, temp);
       iput(fmip);     
       cp += dp->rec_len;  // advance cp by entry_len
       dp = (DIR *)cp;  // pull dp to where cp points at
     }
     // (p 65) with proper typecasting, the last two lines of C code can be simplified as:
     // dp = (DIR *)((char *)dp + dp->rlen);  which eliminates the need for a char *cp
  }
  printf("\n");
}

int ls(char *pathname)  
{
  u32 *ino = malloc(32);
  char currentDir[NMINODE];
  printf("ls %s\n", pathname);
  //findino(running->cwd, ino);

  if(pathname[0] != '\0')  // ls specific directory
  {
  	//getcwd(currentDir, BLKSIZE);  // get current dir
  	strcpy(currentDir, pwd(running->cwd));
  	chdir(pathname);  // get into this dir
  	ls_dir(running->cwd);  // ls this dir
  	chdir(currentDir);  // change back
  	//ino = getino(pathname);
  }	
  else
  	ls_dir(running->cwd);  // ls current dir
}
