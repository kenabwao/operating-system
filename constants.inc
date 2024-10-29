DRIVE_NO                EQU 0x00                ;drive number of disk

LOADER_ADD              EQU 0x1820             ;segment to load stage 2 to
LOADER_OFF              EQU 0x0000             ;offset to load stage 2 to

SECTOR_SIZE             EQU 512
FILE_NAME_SIZE          EQU 7                   ; max is 40
DIRECTORY_ENTRY_SIZE    EQU 64
START_CLUSTER_OFF       EQU 48                 ;offset of pointer to first cluster 
FILE_DATA_SECTOR        EQU 15481               ;(total disk sectors x bytes per cluster / bytes per sector) +  (reserved sectors)
END_OF_FILE             EQU 0xFFFFFFF0