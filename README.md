# hashmap.c

[Hash map](https://en.wikipedia.org/wiki/Hash_table) implementation in C. 

## Features

- [Open addressing](https://en.wikipedia.org/wiki/Hash_table#Open_addressing) using [Robin Hood](https://en.wikipedia.org/wiki/Hash_table#Robin_Hood_hashing) hashing
- Generic interface with support for variable sized items.
- Built-in [SipHash](https://en.wikipedia.org/wiki/SipHash) or [MurmurHash3](https://en.wikipedia.org/wiki/MurmurHash) and allows for alternative algorithms.
- ANSI C (C99)
- Supports custom allocators
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
