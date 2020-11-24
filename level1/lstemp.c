/************* cd_ls_pwd.c file **************/
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int ls_file(char *name)
{
  // READ Chapter 11.7.3 HOW TO ls
  struct stat fstat, *sp;
  int i;
  char ftime[64], l_name[128], buf[BLKSIZE];
  sp = &fstat;
  if((i = lstat(name, &fstat))<0)
  {
  	printf("can't stat %s\n", name);
  	exit(1);
  }
  
  if((sp->i_mode & 0xF000) == 0x8000)  // if (S_ISREG())
  	printf("%c", '-');
  if((sp->i_mode & 0xF000) == 0x4000)  // if (S_ISDIR())
  	printf("%c", 'd');
  if((sp->i_mode & 0xF000) == 0xA000)  // if (S_ISLNK())
  {
  	printf("%c", 'l');
  }
  for(i = 8; i >= 0; i--)
  {
  	if(sp->i_mode & (1<<i))        // print r|w|x
  		printf("%c", t1[i]);
  	else                            // or print -
  		printf("%c", t2[i]);   
  }
  //(p 304)
  printf("%4d ", sp->i_links_count);     // hard-link count
  printf("%4d ", sp->i_gid);              // group ID
  printf("%4d ", sp->i_uid);              // owner uid
  printf("%8d ", sp->i_size);             // file size in bytes
  
  // print time
  //strcpy(ftime, ctime(&ip->i_mtime));  // pass & of time field prints i_atime in calendar form
  //ftime[strlen(ftime)-1] = 0;  // kill \n at end
  //printf("%s  ", ftime);
  strcpy(ftime, ctime(&sp->st_ctime));    // print time in calendar form
  ftime[strlen(ftime)-1] = 0;            // kill \n at end
  printf("%s ", ftime);
  
  // print name
  printf("%s", name);             // print file basename
  
  // print -> linkname if symbolic file
  if((sp->st_mode & 0xF000) == 0xA000)
  {
  	char linkname[BLK];
	i = readlink(fname, linkname, BLK);
  	printf(" -> %s", linkname);
  }
  //iput(mip);
  printf("\n");
}

int ls_dir(char *pathname)
{
  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  MINODE *fmip;
  
  // Assume DIR has only one data block i_block[0]
  dp = opendir(;  // typecasting
  cp = buf;
  
  while (cp < buf + BLKSIZE)
  {
     
  }
  printf("\n");
}

int ls(int argc, char *argv[])  
{
  struct stat mystat, *sp = &mystat;
  char *filename, path[BLKSIZE], cwd[256];
  int r;
  filename = "./";
  
  if(argc > 1)
  	filename = argv[1];
  if(r = lstat(filename, sp)<0)
  {
  	printf("no such file %s\n", filename);
  	exit(1);
  }
  strcpy(path, filename);

  if(path[0] != '/')
  {
  	printf("ls %s\n", path);
  	getcwd(cwd, 256);
  	strcpy(path, cwd); strcat(path, "/"); strcat(path, filename);
  }
  
  if(S_ISDIR(sp->st_mode))
  	ls_dir(path);	
  else
  	ls_file(path);
}
