# hashmap.c

[Hash map](https://en.wikipedia.org/wiki/Hash_table) implementation in C. 

## Features

- [Open addressing](https://en.wikipedia.org/wiki/Hash_table#Open_addressing) using [Robin Hood](https://en.wikipedia.org/wiki/Hash_table#Robin_Hood_hashing) hashing.
- Generic interface with support for variable-sized items.
- Built-in [SipHash](https://en.wikipedia.org/wiki/SipHash), [MurmurHash3](https://en.wikipedia.org/wiki/MurmurHash), [xxHash](https://github.com/Cyan4973/xxHash) and allows for alternative algorithms.
- Supports C99 and up.
- Supports custom allocators.
- Pretty darn good performance. 🚀

## Example

```c
#include <stdio.h>
#include <string.h>
#include "hashmap.h"

struct user {
    char *name;
    int age;
};

int user_compare(const void *a, const void *b, void *udata) {
    const struct user *ua = a;
    const struct user *ub = b;
    return strcmp(ua->name, ub->name);
}

bool user_iter(const void *item, void *udata) {
    const struct user *user = item;
    printf("%s (age=%d)\n", user->name, user->age);
    return true;
}

uint64_t user_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct user *user = item;
    return hashmap_sip(user->name, strlen(user->name), seed0, seed1);
}

int main() {
    // create a new hash map where each item is a `struct user`. The second
    // argument is the initial capacity. The third and fourth arguments are 
    // optional seeds that are passed to the following hash function.
    struct hashmap *map = hashmap_new(sizeof(struct user), 0, 0, 0, 
                                     user_hash, user_compare, NULL, NULL);

    // Here we'll load some users into the hash map. Each set operation
    // performs a copy of the data that is pointed to in the second argument.
    hashmap_set(map, &(struct user){ .name="Dale", .age=44 });
    hashmap_set(map, &(struct user){ .name="Roger", .age=68 });
    hashmap_set(map, &(struct user){ .name="Jane", .age=47 });

    struct user *user; 
    
    printf("\n-- get some users --\n");
    user = hashmap_get(map, &(struct user){ .name="Jane" });
    printf("%s age=%d\n", user->name, user->age);

    user = hashmap_get(map, &(struct user){ .name="Roger" });
    printf("%s age=%d\n", user->name, user->age);

    user = hashmap_get(map, &(struct user){ .name="Dale" });
    printf("%s age=%d\n", user->name, user->age);

    user = hashmap_get(map, &(struct user){ .name="Tom" });
    printf("%s\n", user?"exists":"not exists");

    printf("\n-- iterate over all users (hashmap_scan) --\n");
    hashmap_scan(map, user_iter, NULL);

    printf("\n-- iterate over all users (hashmap_iter) --\n");
    size_t iter = 0;
    void *item;
    while (hashmap_iter(map, &iter, &item)) {
        const struct user *user = item;
        printf("%s (age=%d)\n", user->name, user->age);
    }

    hashmap_free(map);
}

// output:
// -- get some users --
// Jane age=47
// Roger age=68
// Dale age=44
// not exists
// 
// -- iterate over all users (hashmap_scan) --
// Dale (age=44)
// Roger (age=68)
// Jane (age=47)
//
// -- iterate over all users (hashmap_iter) --
// Dale (age=44)
// Roger (age=68)
// Jane (age=47)

```

## Functions

### Basic

```sh
hashmap_new      # allocate a new hash map
hashmap_free     # free the hash map
hashmap_count    # returns the number of items in the hash map
hashmap_set      # insert or replace an existing item and return the previous
hashmap_get      # get an existing item
hashmap_delete   # delete and return an item
hashmap_clear    # clear the hash map
```

### Iteration

```sh
hashmap_iter     # loop based iteration over all items in hash map 
hashmap_scan     # callback based iteration over all items in hash map
```

### Hash helpers

```sh
hashmap_sip      # returns hash value for data using SipHash-2-4
hashmap_murmur   # returns hash value for data using MurmurHash3
```

## API Notes

An "item" is a structure of your design that contains a key and a value.
You load your structure with key and value and you set it in the table,
which copies the contents of your structure into a bucket in the table.
When you get an item out of the table, you load your structure with
the key data and call "hashmap_get()". This looks up the key and returns
a pointer to the item stored in the bucket. The passed-in item is
not modified.

Since the hashmap code doesn't know anything about your item structure,
you must provide "compare" and "hash" functions which access the structure's
key properly. If you want to use the "hashmap_scan()" function, you must also
provide an "iter" function. For your hash function, you are welcome to call
one of the supplied hash functions, passing the key in your structure.

Note that if your element structure contains pointers, those pointer
values will be copied into the buckets. I.e. it is a "shallow" copy
of the item, not a "deep" copy. Therefore, anything your entry points
to must be maintained for the lifetime of the item in the table.

The functions "hashmap_get()", "hashmap_set()", and "hashmap_delete()"
all return a pointer to an item if found.
In all cases, the pointer is not guaranteed to continue to point
to that same item after subsequent calls to the hashmap.
I.e. the hashmap can be rearranged by a subsequent call, which can render
previously-returned pointers invalid, possibly even pointing into freed
heap space. DO NOT RETAIN POINTERS RETURNED BY HASHMAP CALLS!  It is
common to copy the contents of the item into your storage immediately
following a call that returns an item pointer.

NOT THREAD SAFE. If you are using hashmap with multiple threads, you
must provide locking to prevent concurrent calls. Note that it is NOT
sufficient to add the locks to the hashmap code itself. Remember that
hashmap calls return pointers to internal structures, which can become
invalid after subsequent calls to hashmap. If you just add a lock
inside the hashmap functions, by the time a pointer is returned to
the caller, that pointer may have already been rendered invalid.
You should lock before the call, make the call, copy out the result,
and unlock.

## Testing and benchmarks

```sh
$ cc -DHASHMAP_TEST hashmap.c && ./a.out              # run tests
$ cc -DHASHMAP_TEST -O3 hashmap.c && BENCH=1 ./a.out  # run benchmarks
```

The following benchmarks were run on my 2019 Macbook Pro (2.4 GHz 8-Core Intel Core i9) using gcc-9.
The items are simple 4-byte ints. 
The hash function is MurmurHash3. 
Testing with 5,000,000 items.
The `(cap)` results are hashmaps that are created with an inital capacity of 5,000,000.

```
set            5000000 ops in 0.708 secs, 142 ns/op, 7057960 op/sec, 26.84 bytes/op
get            5000000 ops in 0.303 secs, 61 ns/op, 16492723 op/sec
delete         5000000 ops in 0.486 secs, 97 ns/op, 10280873 op/sec
set (cap)      5000000 ops in 0.429 secs, 86 ns/op, 11641660 op/sec
get (cap)      5000000 ops in 0.303 secs, 61 ns/op, 16490493 op/sec
delete (cap)   5000000 ops in 0.410 secs, 82 ns/op, 12200091 op/sec
```

## License

hashmap.c source code is available under the MIT License.
