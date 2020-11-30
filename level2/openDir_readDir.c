int opendir(pathaname)
{
    return open(pathname, RD|O_DIR); 
}

int readdir(int fd, struct udir *udirp) // struct udir{DIR udir;};
{
    // same as read() except:
    // use the current byte offset in OFT to read the next dir_entry;
    // copy the dir_entry into *udirp;
    // advance offset by dir_entry’s rec_len for reading next dir_entry;
}