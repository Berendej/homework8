bayan utility. Purpose: searches files with identical content.

Options:
  -h [ --help ]                  This screen
  -d [ --dir ] arg               directories to scan (several directories 
                                 allowed) current dir by default
  -e [ --excl ] arg              directories to EXCLUDE from scan (several 
                                 directories allowed)
  -r [ --recur ] arg (=0)        scan depth 0-flat , 1-recursively default 0
  -s [ --size ] arg (=1)         minimal file size 1 byte by default
  -m [ --mask ] arg              file name mask
  -b [ --blocksize ] arg (=1024) block size, 1024 bytes by default
  -a [ --algo ] arg (=0)         hash algorithm, 0-md5 , 1-crc32

