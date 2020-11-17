/************************mkdir_creat file**********************************/
//#include <type.h>

int make_dir(char* pathname)
{
   MINODE *start;		   
   int pip, pino;
   char * parent, child;
// 1. pahtname = absolute: start = root;         dev = root->dev;
//             = relative: start = running->cwd; dev = running->cwd->dev;

// 2. Let  pathname = a/b/c
      parent = dirname(pathname);   //parent= "/a/b" OR "a/b"
      child  = basename(pathname);  //child = "c"

//    WARNING: strtok(), dirname(), basename() destroy pathname

// 3. Get minode of parent:

        pino  = getino(parent);
        pip   = iget(dev, pino); 

//    Verify : (1). parent INODE is a DIR (HOW?)   AND
//             (2). child does NOT exists in the parent directory (HOW?);
               
   mymkdir(pip, child);

// 5. inc parent inodes's links count by 1; 
//    touch its atime, i.e. atime = time(0L), mark it DIRTY

   iput(pip);  
}

