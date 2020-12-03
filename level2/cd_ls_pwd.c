/************* cd_ls_pwd.c file **************/
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

char *PWD[NMINODE];
int rpwd(MINODE *wd)
{
  // while(cp<buf+BLKSIZE && x<2)
  // {
  // 	strncpy(temp, dp->name, dp->name_len);
  // 	temp[dp->name_len] = 0;
  	
  // 	if(strcmp(temp, ".") == 0)
  // 	{
  // 		ino = dp->inode;
  // 		x++;
  // 	}
  // 	if(strcmp(temp, "..") == 0)
  // 	{
  // 		parent_ino = dp->inode;
  // 		x++;
  // 	}
  // 	cp += dp->rec_len;
  // 	dp = (DIR *)cp;
  // }
  // pip = iget(dev, parent_ino);
  // findmyname(pip, ino, &my_name);
  // rpwd(pip);
  // printf("/%s", my_name);
  // strcat(PWD, "/");
  // strcat(PWD, my_name);
  //parent_ino = findino(wd, &ino);
  MINODE *pip;
  int parent_ino, my_ino;
  char my_name[256];
  DIR *dp, *mip;
  char *cp;
  char temp[256], buf[BLKSIZE];
  
  if(wd == root)
  	return 0;

  my_ino = search(wd, ".");
  parent_ino = search(wd, "..");
  pip = iget(dev, parent_ino);
  get_block(fd, wd->INODE.i_block[0], buf);
  dp = (DIR*)buf;
  cp = buf;

  while(cp<buf+BLKSIZE)
  {
    strncpy(my_name, dp->name, dp->name_len);
    my_name[dp->name_len] = 0;
    if(dp->inode == my_ino )
    {
      break;
    }
    // move to next dir
    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
  
  rpwd(pip);
  printf("/%s", my_name);
}

int pwd(MINODE *wd)
{
  if (wd == root)
  {	
  	printf("CWD = /\n");
    return 0;
  }
  else
  {
  	rpwd(wd);
    printf("\n");
  }
}

int chdir(char *pathname)   
{
  MINODE *mip;
  int ino;
  
  
  // READ Chapter 11.7.3 HOW TO chdir
  if(pathname == '\0')
  {  	
    //ino = root->ino;
    printf("Change to root\n");
  	running->cwd = root;
  }
  else
  {
    printf("cd %s\n", pathname);
  	ino = getino(pathname);
  	printf("dev=%d ino=%d\n", dev, ino);
  	if(ino == 0)
    	{
      		// does not exist
  		return 0;
    	}  
  	printf("Check if mip is a dir\n");
  	mip = iget(dev, ino);   // set to correct ino #
  	if(S_ISDIR(mip->INODE.i_mode))  // if is a dir
  	{
      		printf("cd success\n");
  		iput(running->cwd);      // release old cwd
  		running->cwd = mip;      // change cwd to mip
  	}
  	else
  	{
  		printf("%s is not a dir\n", pathname);
  		return 0;
  	}
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
  strcpy(ftime, ctime(&ip->i_mtime));  // pass & of time field prints i_atime in calendar form
  ftime[strlen(ftime)-1] = 0;  // kill \n at end
  printf("%s  ", ftime);
  /*strcpy(ftime, ctime(&ip->i_ctime));    // print time in calendar form
  ftime[strlen(ftime)-1] = 0;            // kill \n at end
  printf("%s ", ftime);*/
  
  // print name
  printf("%s", name);             // print file basename
  
  // print -> linkname if symbolic file
  if((ip->i_mode & 0xF000) == 0xA000)
  	printf(" -> %s", l_name);
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
  dp = (DIR *)buf;   // typecasting
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
     }
     cp += dp->rec_len;  // advance cp by entry_len
     dp = (DIR *)cp;     // pull dp to where cp points at
     // (p 65) with proper typecasting, the last two lines of C code can be simplified as:
     // dp = (DIR *)((char *)dp + dp->rlen);  which eliminates the need for a char *cp
  }
  printf("\n");
}

int ls(char *pathname)  
{
  u32 *ino = malloc(32);
  char currentDir[NMINODE];
  //findino(running->cwd, ino);

  if(pathname[0] != '\0')  // ls specific directory
  {
  	printf("ls %s\n", pathname);
  	strcpy(currentDir, pwd(running->cwd));  // get current dir
  	chdir(pathname);  // get into this dir
  	ls_dir(running->cwd);  // ls this dir
  	chdir(currentDir);  // change back
  	//ino = getino(pathname);
  }	
  else
  	ls_dir(running->cwd);  // ls current dir
}
