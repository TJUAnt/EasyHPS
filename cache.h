#ifndef CACHE_H_
#define CACHE_H_
#define page_size 1000
#define cache_table_size 100
#include <map>
#include <string>
#include <map>
#include <bitset>
using namespace std;

typedef long long int addr_idx;
class Cache
{
    private:
	char *filename;
        map<string,addr_idx>addrmp;
        map<string,addr_idx>::iterator itr;
	int addrcnt;
	int cache[page_size*cache_table_size];
        static Cache* cache_ptr;
	pthread_mutex_t *cache_lock[cache_table_size];
        Cache();
        ~Cache();
	bitset<cache_table_size>cache_bit;
	int cache_table_idx[cache_table_size];
	bitset<cache_table_size>cache_modify_flag;
	long long int mem_size;
    public:
	void cache_init();
	void ReadCache(addr_idx idx, int *cache);
        int Read(string str,addr_idx idx);
        void WriteCache(addr_idx file_idx, int *cache);
        void Write(string str, addr_idx idx, int value);
       	static Cache* GetCache();
	void new_mem(string str, addr_idx idx);
};
#endif
