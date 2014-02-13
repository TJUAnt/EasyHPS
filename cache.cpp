#include "cache.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cstdlib>
Cache* Cache::cache_ptr = NULL;

Cache::Cache()
{
    addrcnt = 0;
    mem_size = 0LL;
    addrmp.clear();
    filename = new char [25];
    memset(cache, 0, sizeof(cache));
    memset(cache_table_idx, -1, sizeof(cache_table_idx));
    cache_modify_flag.reset(); 
    cache_bit.reset();
    filename[0] = 'm';
    filename[1] = 'e';
    filename[2] = 'm';
    filename[3] = '/';
    for(int i = 0; i < cache_table_size; i++)
	pthread_mutex_init(cache_lock[i],NULL);
}

Cache::~Cache()
{
    delete filename;
    filename = NULL;
}

Cache* Cache::GetCache()
{
    if(cache_ptr == NULL)
	cache_ptr = new Cache();
    return cache_ptr;
}

void Cache::new_mem(string str, addr_idx idx)
{

    if(addrmp.find(str) == addrmp.end()){
	addrmp[str] = mem_size;
	mem_size += idx;	
	return;
    }
    else{
	fprintf(stderr,"memory new error!\n");
	return;
    }
}


void Cache::ReadCache(addr_idx file_idx,int* cache)
{
    int tt = 4;
    do{ 
	filename[tt++] = 'a' + (file_idx & 0xF);
	file_idx >>= 4;
    }while(file_idx != 0);
    filename[tt] = 0;
    FILE *fp = fopen(filename,"r");
    if(fp != NULL){
	while(fscanf(fp,"%d",cache)!=EOF){ cache++; }
	fflush(fp);
	fclose(fp);
    }
}

void Cache::WriteCache(addr_idx file_idx,int *cache)
{
    if(file_idx == -1)
	return;
    int tt = 4;
    do{ 
	filename[tt++] = 'a' + (file_idx & 0xF);
	file_idx >>= 4;
    }while(file_idx != 0);
    filename[tt] = 0;
    FILE *fp = fopen(filename,"w");
    if(fp == NULL){
	std::cout << "fuck filename" << std::endl;
	fclose(fp);
	return;
    }
    for(int i = 0; i < page_size; i++)
    {
	fprintf(fp,"%d\n",cache[i]);
    }
    fflush(fp);
    fclose(fp);
    return;
}

int Cache::Read(string str, addr_idx idx)
{
    if(addrmp.find(str) == addrmp.end())
	fprintf(stderr, "memory read error!\n");
    idx += addrmp[str];	
    int page_idx = idx / page_size;
    int cache_idx = page_idx % cache_table_size; 
    pthread_mutex_lock(cache_lock[cache_idx]);
    int ret;
    if(cache_table_idx[cache_idx] != page_idx)
    {
	if(cache_modify_flag[cache_idx] == 1){
	    WriteCache(cache_table_idx[cache_idx], cache + page_size * cache_idx);
	    cache_modify_flag[cache_idx] = 0;
	}
	ReadCache(page_idx, cache + page_size*cache_idx);
	cache_table_idx[cache_idx] = page_idx;
    }
    ret = cache[cache_idx*page_size + idx % page_size];
    pthread_mutex_unlock(cache_lock[cache_idx]);
    return cache[cache_idx*page_size + idx % page_size];
}

void Cache::Write(string str, addr_idx idx, int value)
{
    if(addrmp.find(str) == addrmp.end())
	fprintf(stderr, "memory write error!\n");
    idx += addrmp[str];
    int page_idx = idx / page_size;
    int cache_idx = page_idx % cache_table_size; 
    pthread_mutex_lock(cache_lock[cache_idx]);
    if( cache_table_idx[cache_idx] != page_idx )
    {
	if(cache_modify_flag[cache_idx] == 1){
	    WriteCache(cache_table_idx[cache_idx], cache + page_size * cache_idx);
	    cache_modify_flag[cache_idx] = 0;	
	}
	ReadCache(page_idx, cache + page_size*cache_idx);
	cache_table_idx[cache_idx] = page_idx;
    }	
    cache_modify_flag[cache_idx] = 1;
    cache[cache_idx*page_size + idx % page_size] = value; 
    pthread_mutex_unlock(cache_lock[cache_idx]);
}
/*
int main(){
    int loop_count = 1000000;
    Cache* cache = Cache::GetCache();
    srand(0);
    int value;
    string a,b;
    a = "mp";
    b = "dp";
    int idx1,idx2,idx3;
    cache->new_mem(a,loop_count);
    cache->new_mem(b,loop_count);
    for(int i = 0; i < loop_count; i++){
	for(int j = 0; j < 500; j++)
	{
	    cache->Write(a,i*500+j,0);
	} 
    }
    
    for(int i = 0; i < 99; i++){
	cache->Write(a,i*100+i+1,1); 
    }
    for(int k = 0; k < 500; k++){
	for(int i = 0; i < 500; i++)
	{
	    for(int j = 0; j < 500; j++)
	    {
		idx1 = i * 100 + k;
		idx2 = k * 100 + j;
		idx3 = i * 100 + j;
		if(cache->Read(a,idx1)!=0 && cache->Read(a,idx2)!=0){
		    if(cache->Read(a,idx3)==0||cache->Read(a,idx3) > cache->Read(a,idx1) + cache->Read(a,idx2)) 
			cache->Write(a,idx3,cache->Read(a,idx1)+cache->Read(a,idx2)); 
		}
	    }
	}
    }
    for(int i = 0; i < 50; i++)
    {
	for(int j = 0; j < 50; j++)
	    cout << cache->Read(a,i*100+j) << ' ';
	cout << endl;
    }	

    return 0;
}
*/
