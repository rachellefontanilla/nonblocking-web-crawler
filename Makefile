# Makefile, ECE252  
# Rachelle Fontanilla

CC = gcc
CFLAGS_XML2 = $(shell xml2-config --cflags)
CFLAGS_CURL = $(shell curl-config --cflags)
CFLAGS = -Wall $(CFLAGS_XML2) $(CFLAGS_CURL) -g -std=gnu99
LD = gcc
LDFLAGS = -std=gnu99 -g
LDLIBS_XML2 = $(shell xml2-config --libs)
LDLIBS_CURL = $(shell curl-config --libs)
LDLIBS = -lcurl -lz $(LDLIBS_XML2) $(LDLIBS_CURL)

OBJ_DIR = obj
SRC_DIR = lib
LIB_UTIL = $(OBJ_DIR)/curlStuff.o $(OBJ_DIR)/hashHelper.o $(OBJ_DIR)/linkedList.o

SRCS   = web-crawler.c curlStuff.c hashHelper.c linkedList.c
OBJS   = $(OBJ_DIR)/web-crawler.o $(LIB_UTIL) 
TARGETS= web-crawler

all: ${TARGETS}

web-crawler: $(OBJS)
	mkdir -p $(OBJ_DIR)
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

$(OBJ_DIR)/web-crawler.o: web-crawler.c 
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $< 

%.d: %.c
	gcc -MM -MF $@ $<

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *~ *.d *.o all.png *.txt $(TARGETS)