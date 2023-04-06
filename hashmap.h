// Copyright 2020 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file.

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct hashmap;

/**
 * @brief Create a new hashmap
 *
 * @param elsize The size of each element in the tree. Every element that is inserted, deleted or retreived will be this size.
 * @param cap The default lower capacity of the hashmap. Setting this to zero will default to 16.
 * @param seed0 Value passed to the following `hash` function. These can be any value you wish but it's often best to use randomly generated values.(Optional)
 * @param seed1 Value passed to the following `hash` function. These can be any value you wish but it's often best to use randomly generated values.(Optional)
 * @param hash Function that generate a hash value for an item.
 * @param compare Function that compare items in the tree.
 * @param elfree Function that frees a specific item. This should be NULL unless sorting some kind of reference data in the hash
 * @param udata
 *
 * @note It's important to provide a good hash function, otherwise it will perform poorly or be vulnerable to Denial-of-service attacks.
 * @note This implementation comes with two helper functions `hashmap_sip()` and `hashmap_murmur().
 * @return New hashmap
 *
 */
struct hashmap *hashmap_new(size_t elsize, size_t cap,
                            uint64_t seed0, uint64_t seed1,
                            uint64_t (*hash)(const void *item,
                                             uint64_t seed0, uint64_t seed1),
                            int (*compare)(const void *a, const void *b,
                                           void *udata),
                            void (*elfree)(void *item),
                            void *udata);

/**
 * @brief Crate a new hash map using a custom allocator
 * 
 * @note See hashmap_new for more information
 * @return struct hashmap* 
 */
struct hashmap *hashmap_new_with_allocator(
    void *(*malloc)(size_t),
    void *(*realloc)(void *, size_t),
    void (*free)(void *),
    size_t elsize, size_t cap,
    uint64_t seed0, uint64_t seed1,
    uint64_t (*hash)(const void *item,
                     uint64_t seed0, uint64_t seed1),
    int (*compare)(const void *a, const void *b,
                   void *udata),
    void (*elfree)(void *item),
    void *udata);

/**
 * @brief Free the hash map after usage
 * 
 * @param map Pointer to the hash map to be freed 
 */
void hashmap_free(struct hashmap *map);

/**
 * @brief Quicly clears the map.
 * 
 * @param map Hash map to be cleared 
 * @param update_cap If provided the map's capacity will be updated to match the currently number of allocated buckets.
 * @note This is an optimization to ensure that this operation does not perform any allocations.
 */
void hashmap_clear(struct hashmap *map, bool update_cap);

/**
 * @brief 
 * 
 * @param map Hash map to get the size of.
 * @return Return the number of items in the hash map 
 */
size_t hashmap_count(struct hashmap *map);

/**
 * @brief Function that checks for memeory when using the hashmap_set() function.
 * 
 * @param map Hash map in the subject 
 * @return true if the last  hashmap_set() call failed due to memory leak.
 * @return false if the last hashmap_set() call is successful.
 */
bool hashmap_oom(struct hashmap *map);

/**
 * @brief Get the item  based on the provided key
 * 
 * @param map Hash map to look in it 
 * @param item Item to look for in the provided hash map
 * @return NULL if the item is not found
 */
void *hashmap_get(struct hashmap *map, const void *item);

/**
 * @brief Insert or replace an item in the hash map.
 * 
 * @param map Hash map to insert into. 
 * @param item Item to be inserted.
 * @return If item is replaced it is returned else NULL
 * @note If the system is unable to allocate memory then NULL is returned and the hashmap_oom() return true.
 */
void *hashmap_set(struct hashmap *map, const void *item);

/**
 * @brief Remove an item from the hash map
 * 
 * @param map Hash map to delete from.
 * @param item Item to delete from the hash map.
 * @return Item removed from the hash map. If not found returns NULL
 */
void *hashmap_delete(struct hashmap *map, void *item);

/**
 * @brief Returns the item in the bucket at position or NULL if an item is not set for that bucket.
 * 
 * @param map Hash map to look in it.
 * @param position Index of the element to extract.
 */
void *hashmap_probe(struct hashmap *map, uint64_t position);

/**
 * @brief Iterates over all items in the hash map.
 * 
 * @param map Hash map to iterate over
 * @param iter Can return false to stop iteration early.
 * @param udata 
 * @return true if the iteration is fully completed, false if the iteration is stopped early.  
 */
bool hashmap_scan(struct hashmap *map,
                  bool (*iter)(const void *item, void *udata), void *udata);

/**
 * @brief iterates one key at a time yielding a reference to an
    entry at each iteration. Useful to write simple loops and avoid writing
    dedicated callbacks and udata structures, as in hashmap_scan.
 * 
 * @param map Hash map handle
 * @param i Pointer to a size_t cursor that should be initialized to 0 at the beginning of the loop.
 * @param item Void pointer pointer that is populated with the retrieved item.
 * @return true if an item was received
 * @return false if the end of the iteration was reached.
 */
bool hashmap_iter(struct hashmap *map, size_t *i, void **item);

/**
 * @brief Return a hash value for `data` using SipHash-2-4
 * 
 */
uint64_t hashmap_sip(const void *data, size_t len,
                     uint64_t seed0, uint64_t seed1);

/**
 * @brief Returns a hash value for `data` using Murmur3_86_128.
 */
uint64_t hashmap_murmur(const void *data, size_t len,
                        uint64_t seed0, uint64_t seed1);

// DEPRECATED: use `hashmap_new_with_allocator`
void hashmap_set_allocator(void *(*malloc)(size_t), void (*free)(void *));

#endif
