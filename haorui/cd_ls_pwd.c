/************* cd_ls_pwd.c file **************/
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int chdir(char *pathname)   
{
  MINODE *mip;
  int ino;
  
  printf("cd %s\n", pathname);
  // READ Chapter 11.7.3 HOW TO chdir
  if(pathname != 0)
  	ino = getino(pathname);
  else
  	ino = root->ino;
  	
  mip = iget(dev, ino);
  if(S_ISDIR(mip->INODE.i_mode))  // if is a dir
  {
  	iput(running->cwd);      // release old cwd
  	running->cwd = mip;      // change cwd to mip
  }
}

int ls_file(MINODE *mip, char *name)
{
  // READ Chapter 11.7.3 HOW TO ls
  int i;
  char ftime[64], l_name[128], buf[BLKSIZE];
  INODE *ip = &mip->INODE;
  
  if((ip->i_mode & 0xF000) == 0x8000)  // if (S_ISREG())
  	printf("%c", '-');
  if((ip->i_mode & 0xF000) == 0x4000)  // if (S_ISDIR())
  	printf("%c", 'd');
  if((ip->i_mode & 0xF000) == 0xA000)  // if (S_ISLNK())
  {
  	printf("%c", 'l');
  	get_block(mip->dev, mip->INODE.i_block[0], buf);
  	strcpy(l_name, buf);
  	put_block(mip->dev, mip->INODE.i_block[0], buf);
  	l_name[strlen(l_name)] = 0;
  }
  for(i = 8; i >= 0; i--)
  {
  	if(ip->i_mode & (1<<i))        // print r|w|x
  		printf("%c", t1[i]);
  	else                            // or print -
  		printf("%c", t2[i]);   
  }
  //(p 304)
  printf("%4d ", ip->i_links_count);     // hard-link count
  printf("%4d ", ip->i_gid);              // group ID
  printf("%4d ", ip->i_uid);              // owner uid
  printf("%8d ", ip->i_size);             // file size in bytes
  
  // print time
  printf("%s", ctime(&ip->i_atime));  // pass & of time field prints i_atime in calendar form
  /*strcpy(ftime, ctime(&ip->i_ctime));    // print time in calendar form
  ftime[strlen(ftime)-1] = 0;            // kill \n at end
  printf("%s ", ftime);*/
  
  // print name
  printf("%s", name);             // print file basename
  
  // print -> linkname if symbolic file
  if((ip->i_mode & 0xF000) == 0xA000)
  	printf(" -> %s", l_name);
  iput(mip);
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
  dp = (DIR *)buf;
  cp = buf;
  
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
     printf("[%d %s]  ", dp->inode, temp); // print [inode# name]
     
     fmip = iget(dev, dp->inode);
     fmip->dirty = 0;
     ls_file(fmip, temp);
     
     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");
  iput(mip);
}

int ls(char *pathname)  
{
  printf("%s", pathname);
  u32 *ino = malloc(32);
  printf("Entering ls.\n");
  findino(running->cwd, ino);
  printf("ls %s\n", pathname);
  if(pathname != 0)
  	ino = getino(pathname);
  	
  if(ino != 0)
  ls_dir(iget(dev, *ino));
  //ls_dir(running->cwd);
}

char *rpwd(MINODE *wd)
{
  MINODE *pip;
  int parent_ino;
  u32 ino;
  char my_name[256];
  
  if(wd == root)
  	return;
  	
  parent_ino = findino(wd, &ino);
  pip = iget(dev, parent_ino);
  findmyname(pip, ino, my_name);
  rpwd(pip);
  printf("/%s", my_name);
}

char *pwd(MINODE *wd)
{
  if (wd == root)
  	printf("/\n");
  else
  	rpwd(wd);
}
