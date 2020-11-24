int chdir(char *pathname)   
{
  MINODE *mip;
  int ino;
  
  printf("cd %s\n", pathname);
  // READ Chapter 11.7.3 HOW TO chdir
  if(pathname == '\0')
  {  	
    	//ino = root->ino;
  	running->cwd = root;
  }
  else
  {
  	ino = getino(pathname);  // get inode number of pathname
  	printf("dev=%d ino=%d\n", dev, ino);
  	if(ino == 0)   // does not exist
  		return 0;
  	
  	mip = iget(dev, ino);   // set to correct ino #
  	if(S_ISDIR(mip->INODE.i_mode))  // if is a dir
  	{
  		iput(running->cwd);      // release old cwd
  		running->cwd = mip;      // change cwd to mip
  	}
  	else
  	{
  		printf("%s is not a dir\n", pathname);
  		return 0;
  	}
  		
  	//rpwd(running->cwd);
  }
}
