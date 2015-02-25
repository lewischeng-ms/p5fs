# p5 file system implmentation
# By Lewis Cheng @2010/5/8

CC := gcc -DNDEBUG
BIN := ../bin
SRC := .
OBJ := ../obj
OUT := $(BIN)/testp5

P5_H := p5.h
CACHE_H := cache.h
INODE_H := inode.h
ZONE_H := zone.h
DZ_H := dz.h
PATH_H := path.h
BITMAP_H := bitmap.h

TESTP5_C := testp5.c
BLOCK_C := block.c
FILE_C := file.c
DIR_C := dir.c
CACHE_C := cache.c
INODE_C := inode.c
ZONE_C := zone.c
DZ_C := dz.c
PATH_C := path.c
FS_C := fs.c
BITMAP_C := bitmap.c

VPATH := $(OBJ)

OBJS := $(OBJ)/block.o $(OBJ)/file.o $(OBJ)/dir.o $(OBJ)/cache.o $(OBJ)/inode.o $(OBJ)/zone.o $(OBJ)/dz.o $(OBJ)/path.o $(OBJ)/fs.o $(OBJ)/bitmap.o

TARGETS := block.o file.o dir.o cache.o inode.o zone.o dz.o path.o fs.o bitmap.o

.PHONY: testp5 clean

testp5: $(TARGETS) testp5.o
	$(CC) -O2 -o $(OUT) $(OBJS) $(OBJ)/testp5.o
	cp $(OUT) ./a
testp5.o: $(P5_H) $(TESTP5_C)
	$(CC) -o $(OBJ)/testp5.o -c $(TESTP5_C)
block.o: $(P5_H) $(BLOCK_C)
	$(CC) -o $(OBJ)/block.o -c $(BLOCK_C)
file.o: $(P5_H) $(FILE_C)
	$(CC) -o $(OBJ)/file.o -c $(FILE_C)
dir.o: $(P5_H) $(DIR_C)
	$(CC) -o $(OBJ)/dir.o -c $(DIR_C)
cache.o: $(P5_H) $(CACHE_H) $(CACHE_C)
	$(CC) -o $(OBJ)/cache.o -c $(CACHE_C)
inode.o: $(P5_H) $(INODE_H) $(INODE_C)
	$(CC) -o $(OBJ)/inode.o -c $(INODE_C)
zone.o: $(P5_H) $(ZONE_H) $(ZONE_C)
	$(CC) -o $(OBJ)/zone.o -c $(ZONE_C)
dz.o: $(P5_H) $(DZ_H) $(DZ_C)
	$(CC) -o $(OBJ)/dz.o -c $(DZ_C)
path.o: $(P5_H) $(PATH_H) $(PATH_C)
	$(CC) -o $(OBJ)/path.o -c $(PATH_C)
fs.o: $(P5_H) $(FS_C)
	$(CC) -o $(OBJ)/fs.o -c $(FS_C)
bitmap.o: $(P5_H) $(BITMAP_H) $(BITMAP_C)
	$(CC) -o $(OBJ)/bitmap.o -c $(BITMAP_C)
clean:
	rm -rf $(OBJS) $(OBJ)/testp5.o
	rm -rf $(OUT)
	rm -rf $(OUT1)
	rm -rf a
