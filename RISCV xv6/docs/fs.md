1) /dev/nvme0n1p1    
2)
* brw-rw---- 1 root disk 259, 1 Apr 22 19:23 /dev/nvme0n1p1
* it has a 'b' flag in the permissions, also in my terminal it is highlighted
3) 104857600 bytes (105 MB, 100 MiB) copied, 0.08886 s, 1.2 GB/s
4) Creating filesystem with 102400 1k blocks and '25584 inodes'
5) (block count / free blocks) * 100 -> (90,319 / 102,400) * 100 -> ~88.2%
6)  2  (12) .    2  (12) ..    11  (988) lost+found
7) 
* total 4
-rw-r--r-- 1 drshubov students  0 May  5 14:00 fileA
-rw-rw-rw- 1 drshubov students  0 May  5 14:00 fileB
---------- 1 drshubov students  0 May  5 14:00 fileC
-rw------- 1 drshubov students 36 May  5 14:00 whoami.txt
* umask acts as a mask that subtracts permissions
8) 773
9) 9799482871
10)
* ls opens the directory and gets an fd
* if valid, it continously reads the contents/entries of the dir (while loop)
* prints out the parts from 'stat', using the corresponding inode to get relevant metadata

```
// On-disk inode structure
 struct dinode {
   short type;           // File type
   short major;          // Major device number (T_DEVICE only)
   short minor;          // Minor device number (T_DEVICE only)
   short nlink;          // Number of links to inode in file system
   uint size;            // Size of file (bytes)
   uint addrs[NDIRECT+1];   // Data block addresses
 };
```

                          
