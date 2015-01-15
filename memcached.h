#include<stdio.h>
#include<libmemcached/memcached.h>
#include<string.h>

memcached_st *mc;
memcached_server_st *servers;
memcached_return rc;
time_t expire;
uint32_t flags;


void mc_connect();
void mc_free();

void mc_set(const char *key, const char *val, const time_t expire);
char *mc_get(const char *key);
void mc_del(const char *key, const time_t expire);

