CC=cc
LD=ld
CFLAGS= -g -D_REENTRANT=1 -m32 -O2 -Wall
LDFLAGS= -g -shared -m32

TS_BASE=../../../..
API_INC= -I. -I$(TS_BASE)/proxy/api/include

default: redis_cache.so

credis.o: credis.c
	$(CC) $(CFLAGS) $(API_INC) -o credis.o -c credis.c

rediscache_filter.o: rediscache_filter.cc
	$(CXX) $(CFLAGS) $(API_INC) -o rediscache_filter.o -c rediscache_filter.cc

rediscache_filter.so: rediscache_filter.o credis.o
	$(CXX) $(LDFLAGS) -o rediscache_filter.so rediscache_filter.o credis.o

clean:
	rm -f rediscache_filter.so rediscache_filter.o credis.o
