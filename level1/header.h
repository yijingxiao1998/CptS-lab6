#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"

// globals
MINODE minode[NMINODE];
MINODE *root;

// An OS kernel usually defines a finite number of PROC structures in its data area to represent processes in the system
// Global PROC pointer running to point at the PROC that is currently executing
PROC   proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[32];  // assume at most 32 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;

MINODE *iget();

#include "util.c"
#include "rmdir.c"
#include "pwd.c"
#include "cd.c"
#include "ls.c"
#include "quit.c"
#include "mkdir.c"
#include "creat.c"
#include "alloc_dealloc.c"
//#include "link_unlink_symlink.c"
