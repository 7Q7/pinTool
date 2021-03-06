#include <stdlib.h>

#define SET_BITS 12
#define SET_NUM 4096
#define SET_SIZE 8
#define BLOCK_SIZE 64

#define CACHE_HIT 1
#define MISS_NO_EVICT 0
#define MISS_NON_DIRTY_EVICT 2
#define MISS_DIRTY_EVICT 3

#define READ_OP 0
#define WRITE_OP 1

typedef unsigned long long addr_t;

struct Memblock {
    addr_t addr_line;
    long long recency_val;
    int is_dirty;
};

inline addr_t MissCheck(const int ins_op, const addr_t block_addr);
inline void InitMask();
inline void InitSet_Usage();

struct Memblock lru_cache[SET_NUM][SET_SIZE] = { 0 };
int set_usage[SET_NUM];
addr_t set_mask;

inline void InitMask() {

   set_mask = 0x0;
   for (int i = 0; i < SET_BITS; i++) {
	set_mask = set_mask | (0x1 << i );
   }
}

inline void InitSet_Usage() {
  for(int i = 0; i < SET_NUM; ++i)
    set_usage[i] = -1;
}

inline addr_t CacheCall(const int ins_op, const addr_t block_addr, addr_t* evict_addr) {

  //#define CACHE_HIT 1
  //#define MISS_NO_EVICT 0
  //#define MISS_NON_DIRTY_EVICT 2
  //#define MISS_DIRTY_EVICT 3 
  int evict_type = MISS_NO_EVICT;
  
  *evict_addr = MISS_NO_EVICT;
  addr_t cache_set = block_addr & set_mask;
  
  int usage_index = set_usage[cache_set];

  bool is_hit = false;
  for(int i = 0; i <= usage_index; i++)
    {
      lru_cache[cache_set][i].recency_val++;
      if(lru_cache[cache_set][i].addr_line == block_addr)
	{
	  lru_cache[cache_set][i].recency_val = 0;

	  // if(ins_op) is true for WRITE_OP
	  if(ins_op)
	    {
	      lru_cache[cache_set][i].is_dirty = 1;
	    }
	  is_hit = true;
	}
    }

  if(is_hit)
    return CACHE_HIT;

  //OR evict_type = CACHE_HIT

  if (usage_index < SET_SIZE - 1)
    {
      // Insertion
      usage_index++;
      set_usage[cache_set] = usage_index;
      lru_cache[cache_set][usage_index].addr_line = block_addr;
      lru_cache[cache_set][usage_index].recency_val = 0;

      // Bring in cache and mark dirty for WRITE_OP
      if(ins_op)
	{
	  lru_cache[cache_set][usage_index].is_dirty = 1;
	}

      return MISS_NO_EVICT;
	
    }
  else
    {
	int lruIdx = -1;
	long long largestRecencyVal = -1;
	for(int i = 0; i <= usage_index; i++)
	  {
	    //std::cout << lru_cache[cache_set][i].recency_val << " > " << lru << "\n";
	    if(lru_cache[cache_set][i].recency_val > largestRecencyVal)
	      {
		lruIdx = i;
		largestRecencyVal = lru_cache[cache_set][i].recency_val;
	      }
	  }


	// an address is _always_ returned on eviction
	*evict_addr = lru_cache[cache_set][lruIdx].addr_line;

	if(lru_cache[cache_set][lruIdx].is_dirty)
	  {
	    // dirty block is evicted
	    evict_type = MISS_DIRTY_EVICT;
	  }
	else
	  {
	    // non-dirty block evicted
	    evict_type = MISS_NON_DIRTY_EVICT;
	  }

	
	lru_cache[cache_set][lruIdx].addr_line = block_addr;
	lru_cache[cache_set][lruIdx].recency_val = 0;

	if(ins_op)
	  {
	    // marking inserted block as dirty if instruction = WRITE
	    lru_cache[cache_set][lruIdx].is_dirty = 1;
	  }
	else
	  {
	    // marking inserted  block as non-dirty if insturction = READ
	    lru_cache[cache_set][lruIdx].is_dirty = 0;
	  }
	
	return evict_type;
    }

	
 
}
