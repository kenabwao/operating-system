file_name       db  'loader', 0                                 ;stage 2 file name
no_file         db  'File not found, press anykey', 13, 10, 0   ;error message if file is not found
disk_err        db  'Disk error', 13, 10, 0                     ;error accessing disk
