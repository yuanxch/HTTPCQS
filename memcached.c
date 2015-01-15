#include "memcached.h"

void mc_connect()
{	
	mc = memcached_create(NULL);
	servers = memcached_servers_parse("10.96.141.72:11210");
	//servers = memcached_server_list_append(NULL, "10.96.141.72", 11210, &rc);
	rc = memcached_server_push(mc, servers);

	if(rc != MEMCACHED_SUCCESS) {
		perror("memcached_server_push fail:");
	}
}


void mc_free()
{
	memcached_server_list_free(servers);
}

char *mc_get(const char *key)
{
	size_t key_len = (size_t)strlen(key);
	size_t val_len = 0;

	char *result = memcached_get(mc, key, key_len, &val_len, &flags, &rc);
	
	//if(rc != MEMCACHED_SUCCESS) {
	//	fprintf(stderr, "memcached_get %s fail\n", key);
	//} 

	return result;
}

void mc_set(const char *key, const char *val, const time_t expire)
{
	rc = memcached_set(mc, key, strlen(key), val, strlen(val), expire, flags);

	if(rc != MEMCACHED_SUCCESS) {
		fprintf(stderr, "memcached_set\t%s\t%s\n fail!\n", key, val);
	}
}


void mc_del(const char *key, const time_t expire)
{
	rc = memcached_delete(mc, key, strlen(key), expire);

	if(rc != MEMCACHED_SUCCESS) {
		fprintf(stderr, "memcached_delete\t%s\n Fail!", key);
	}
}



/*
int main()
{
	char *key = "name";
	char *val = "zhangsan";
	char *result;

	mc_connect();

	//set
	mc_set(key, val, 30);
	result = mc_get(key);
	printf("get\t%s\t%s\n", key, result);

	//del
	mc_del(key, 0);
	result = mc_get(key);
	printf("get\t%s\t%s\n", key, result);

	mc_free();

	return 0;
}
*/