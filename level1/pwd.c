char *pwdtemp[NMINODE];

char *rpwd(MINODE *wd)
{
  MINODE *pip;
  int parent_ino, x = 0;
  u32 ino;
  char my_name[256];
  DIR *dp;
  char *cp;
  char temp[256], buf[BLKSIZE];
  
  if(wd == root)
  	return;
  	
  get_block(dev, wd->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  
  while(cp<buf+BLKSIZE && x<2)
  {
   	strncpy(temp, dp->name, dp->name_len);
   	temp[dp->name_len] = 0;
  	
   	if(strcmp(temp, ".") == 0)
   	{
   		ino = dp->inode;
   		x++;
  	}
   	if(strcmp(temp, "..") == 0)
   	{
   		parent_ino = dp->inode;
   		x++;
   	}
   	cp += dp->rec_len;
   	dp = (DIR *)cp;
  }
  pip = iget(dev, parent_ino);
  findmyname(pip, ino, my_name);
  rpwd(pip);
  printf("/%s", my_name);
  strcat(pwdtemp, "/");
  strcat(pwdtemp, my_name);

  /*parent_ino = findino(wd, &ino);
  pip = iget(dev, parent_ino);
  findmyname(pip, ino, &my_name);
  rpwd(pip);
  printf("/%s", my_name);*/
}

char *pwd(MINODE *wd)
{
  strcpy(pwdtemp, "");
  if (wd == root)
  {	
  	printf("CWD = /\n");
  	return "/";
  }
  else
  {
  	printf("CWD = ");
  	rpwd(wd);
  	printf("\n");
  	return pwdtemp;
  }
}
