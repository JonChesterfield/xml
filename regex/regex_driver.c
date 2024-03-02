#include "regex_driver.h"
#include "regex.h"
#include "../tools/arena.libc.h"
#include "../tools/contract.h"
#include "../tools/stack.libc.h"
#include "regex_string.h"
#include "regex.ptree.h"

static const uint64_t hashderiv_sentinel = UINT64_MAX;

#define DRIVER_CONTRACTS 1

static const struct arena_module_ty arena_libc_contract =
    ARENA_MODULE_INIT(arena_libc,
#if DRIVER_CONTRACTS
                      contract_unit_test
#else
                      0
#endif
    );

static arena_module arena_mod = &arena_libc_contract;

static inline arena_t hashderiv_hash_to_arena(hashtable_t h) {
  arena_t a;
  a.base = h.state[0];
  a.next = h.state[1];
  a.limit = h.state[2];
  return a;
}

static inline hashtable_t hashderiv_arena_to_hash(arena_t a) {
  hashtable_t h;
  h.state[0] = a.base;
  h.state[1] = a.next;
  h.state[2] = a.limit;
  h.state[3] = 0;
  return h;
}

// Layout could be an array of keys followed by an array of value, or
// an array of {key, values}
// The former is faster here as values is larger than cache lines
// Storing size in the arena next/allocator position

enum {
  bytes_key_size = 8,
  bytes_value_size = 8 * 256,
  element_size = bytes_key_size + bytes_value_size,
};


static inline hashtable_t hashderiv_create(uint64_t size) {
  arena_t a = arena_create(arena_mod, element_size * size);
  unsigned char *p = arena_base_address(arena_mod, a);
  
  for (uint64_t i = 0; i < (size * element_size)/8; i++) {
    // Keys placed at the start, but going to write -0 across the whole thing
    unsigned char *c = p + 8 * i;
    __builtin_memcpy(c, &hashderiv_sentinel, bytes_key_size);
  }

  return hashderiv_arena_to_hash(a);
}

static inline void hashderiv_destroy(hashtable_t h) {
  arena_destroy(arena_mod, hashderiv_hash_to_arena(h));
}

static inline bool hashderiv_valid(hashtable_t h) {
  return arena_valid(arena_mod, hashderiv_hash_to_arena(h));
}

static inline void hashderiv_store_userdata(hashtable_t *h, uint64_t v) {
  h->state[3] = v;
}

static inline uint64_t hashderiv_load_userdata(hashtable_t *h) {
  return h->state[3];
}

static uint64_t hashderiv_key_hash(hashtable_t h, unsigned char *bytes) {
  (void)h;
  // identity at present
  uint64_t r;
  __builtin_memcpy(&r, bytes, 8);
  return r;
}

static bool hashderiv_key_equal(hashtable_t h, const unsigned char *left,
               const unsigned char *right) {
  (void)h;
  return __builtin_memcmp(left, right, 8) == 0;
}

// Keys and values are the same length, store size in the arena metadata
static uint64_t hashderiv_size(hashtable_t h) {                            
  arena_t a = hashderiv_hash_to_arena(h);                                  
  uint64_t allocation_edge = (char *)arena_next_address(arena_mod, a) -          
    (char *)arena_base_address(arena_mod, a);           
  return allocation_edge / 8;                                                
}                                                                            

static void hashderiv_set_size(hashtable_t *h, uint64_t s) {              
  arena_t a = hashderiv_hash_to_arena(*h);                                 
  arena_change_allocation(arena_mod, &a, s * 8);                             
  *h = hashderiv_arena_to_hash(a);                                         
}                                                                            

static uint64_t hashderiv_capacity(hashtable_t h) {                        
  arena_t a = hashderiv_hash_to_arena(h);                                  
  return arena_capacity(arena_mod, a) / element_size;                             
}


static uint64_t hashderiv_available(hashtable_t h) {                       
  return hashderiv_capacity(h) - hashderiv_size(h);                      
}                                                                            


static unsigned char *hashderiv_location_key(hashtable_t h,                
                                                 uint64_t offset) {            
  arena_t a = hashderiv_hash_to_arena(h);                                  
  unsigned char *p = arena_base_address(arena_mod, a);                           
  return p + offset * bytes_key_size;                                                     
  }                                                                            

static unsigned char *hashderiv_location_value(hashtable_t h,                
                                               uint64_t offset) {            
  arena_t a = hashderiv_hash_to_arena(h);                                  
  unsigned char *p = arena_base_address(arena_mod, a);
  uint64_t capacity = hashderiv_capacity(h);

  // Step over the keys
  p += capacity * bytes_key_size;
  
  return p + offset * bytes_value_size;                                                  
}                                                                            

static uint64_t hashderiv_lookup_offset(hashtable_t h,                     
                                            unsigned char *key) {              
  uint64_t hash = hashderiv_key_hash(h, key);                                          
  uint64_t cap = hashderiv_capacity(h);                                    
                                                                               
#if DRIVER_CONTRACTS
      contract_unit_test(contract_is_power_of_two(cap), "cap offset", 10);               
#endif
                                                                               
    const uint64_t mask = cap - 1;                                             
    const uint64_t start_index = hash;                                         
                                                                               
    for (uint64_t c = 0; c < cap; c++) {                                       
      uint64_t index = (start_index + c) & mask;                               
                                                                               
      unsigned char *loc_key = hashderiv_location_key(h, index);             
                                                                               
      if (hashderiv_key_equal(h, loc_key, key)) {                                        
        /* Found key */                                                        
        return index;                                                          
      }                                                                        
                                                                               
      if (hashderiv_key_equal(h, loc_key, (unsigned char *)&hashderiv_sentinel)) {     
        /* Found a space */                                                    
        return index;                                                          
      }                                                                        
    }                                                                          
                                                                               
#if DRIVER_CONTRACTS
      contract_unit_test(hashderiv_available(h) == 0, "avail 0", 7);                   
#endif
    return UINT64_MAX;                                                         
}


static const struct hashtable_module_ty hashtable_mod_state = {
    .create = hashderiv_create,
    .destroy = hashderiv_destroy,
    .valid = hashderiv_valid,
    .store_userdata = hashderiv_store_userdata,
    .load_userdata = hashderiv_load_userdata,
    .key_align = 8,
    .key_size = bytes_key_size,
    .value_align = 8,
    .value_size = bytes_value_size,
    .key_hash = hashderiv_key_hash,
    .key_equal = hashderiv_key_equal,
    .sentinel = (const unsigned char *)&hashderiv_sentinel,
    .size = hashderiv_size,
    .capacity = hashderiv_capacity,
    .lookup_offset = hashderiv_lookup_offset,
    .location_key = hashderiv_location_key,
    .location_value = hashderiv_location_value,
    .set_size = hashderiv_set_size,
    .maybe_remove = 0,
#if INTSET_CONTRACTS
    .maybe_contract = contract_unit_test,
#else
    .maybe_contract = 0,
#endif
};

static const hashtable_module hashtable_mod = &hashtable_mod_state;

regex_driver_t regex_driver_create(void) {
  regex_driver_t d;
  d.regex_to_derivatives = hashtable_create(hashtable_mod, 16);
  d.strtab = stringtable_create();
  return d;
}

void regex_driver_destroy(regex_driver_t d) {
  hashtable_destroy(hashtable_mod, d.regex_to_derivatives);
  stringtable_destroy(d.strtab);
}

bool regex_driver_valid(regex_driver_t d) {
  return hashtable_valid(hashtable_mod, d.regex_to_derivatives) &&
         stringtable_valid(d.strtab);
}

bool regex_driver_insert(regex_driver_t * driver, const char * bytes, size_t N)
{
  const bool verbose = true;

  printf("Driver insert\n");
  
  if (!regex_in_byte_representation(bytes, N)) {
    return false;
  }

  // might be better on the heap, 2k on the stack is borderline
  stringtable_index_t derivatives[256];
  
  // Nul terminate it explicitly in case N accidentally missed the trailing 0
  stringtable_index_t first = stringtable_insert(&driver->strtab, bytes, N);
  if (!stringtable_index_valid(first)) { return false; }
  
  unsigned char zeros[1] = {0};
  if (!arena_append_bytes(driver->strtab.arena_mod, &driver->strtab.arena, &zeros[0],1)) {
    return false;
  }
    
  void * stack = stack_create(&stack_libc, 8);
  if (!stack) { return false; }
  stack_push_assuming_capacity(&stack_libc, stack, first.value);


  printf("Driver insert main loop\n");
  uint64_t counter = 0;
  while (stack_size(&stack_libc, stack) != 0)
    {
      // printf("Driver insert iteration %lu\n", counter++);
      
      stringtable_index_t active = {.value = stack_pop(&stack_libc, stack),};

      // Check if in the derivative table already
      if (hashtable_contains(hashtable_mod, driver->regex_to_derivatives, (unsigned char*)&active.value))
        {
          // printf("regex already split: %s\n",stringtable_lookup(&driver->strtab, active));
          continue;
        }

      // Not currently an arena, so create and tear down repeatedly
      ptree_context ptree_ctx = regex_ptree_create_context();

      
      const char * regex = stringtable_lookup(&driver->strtab, active);
      if (!regex) { return false; }
      size_t N = __builtin_strlen(regex);;

      printf("  Try building a regex from %s\n", regex);
      ptree tree = regex_from_char_sequence(ptree_ctx, regex, N);
      if (ptree_is_failure(tree)) { return false; }

      tree = regex_canonicalise(ptree_ctx, tree);
      if (ptree_is_failure(tree)) { return false; }

      {
        void * r = stack_reserve( &stack_libc, stack, stack_size(&stack_libc, stack) + 256);
        if (!r) { return false; }
        stack = r;
      }

      for (size_t i = 0; i < 256; i++)
        {
          // Take derivative
          ptree ith = regex_derivative(ptree_ctx, tree, (uint8_t)i);
          if (ptree_is_failure(ith)) { return false; }

          // Canonicalise it
          ith = regex_canonicalise(ptree_ctx, ith);

          // Serialise into the stringtable
          uint64_t offset_before = arena_next_offset(driver->strtab.arena_mod, driver->strtab.arena);
          int rc = regex_to_char_sequence(driver->strtab.arena_mod, &driver->strtab.arena, ith);
          if (rc != 0) { return false; }
          if (!arena_append_bytes(driver->strtab.arena_mod, &driver->strtab.arena, &zeros[0], 1)) {
            return false;
          }
          uint64_t offset_after = arena_next_offset(driver->strtab.arena_mod, driver->strtab.arena);
          stringtable_index_t incr =
            stringtable_record(&driver->strtab, offset_after - offset_before);
          if (!stringtable_index_valid(incr)) { return false; }

          // Store the deduplicated one in the derivatives table          
          derivatives[i] = incr;

          // And push it on the stack
          stack_push_assuming_capacity(&stack_libc, stack, incr.value);                    
        }

      if (hashtable_available(hashtable_mod,
                              driver->regex_to_derivatives) < 8)
        {
          if (!hashtable_rehash_double(hashtable_mod,
                                       &driver->regex_to_derivatives))
            {
              return false;
            }
        }
      
      hashtable_insert(hashtable_mod,
                       &driver->regex_to_derivatives,
                       (unsigned char*)&active.value,
                       (unsigned char*)&derivatives[0]);
      
      regex_ptree_destroy_context(ptree_ctx);
    }
  
  stack_libc_destroy(stack);

  return true;
}
