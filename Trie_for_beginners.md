# Trie for Beginners

## Best way to understand what is Trie in understanding how several keys will be arranged inside difference associative arrays. We have only three keys - each with fourth segments (one segment could be 1 byte or 4 byte or 8 bytes - it doesn't matter in our explanation).
<img src="https://s16.postimg.org/d9tremns5/keys.png" width="50%" height="50%" />

## In Binary Tree these keys will be arranged as ordered data
<img src="https://s16.postimg.org/x35v7c15x/inside_binary_tree.png" width="50%" height="50%" />

## In Hashtable these keys will be arranged in more complex structure as unordered data. Also sometimes it needs more extra space.
<img src="https://s16.postimg.org/6tksojf8l/inside_hashtable.png" width="50%" height="50%" />

## In Trie these keys will be arranged as ordered data and sometimes it's need a little bit less space.
<img src="https://s16.postimg.org/uw1mjevvp/inside_trie.png" width="50%" height="50%" />

## OK. What happens if we want insert a new key into each structure ?
<img src="https://s16.postimg.org/wmknkwdet/new_key.png" width="50%" height="50%" />

## In Binary Tree we need find right place in ordered sequence and insert our key here. Sometimes we need split our page to do balancing of tree.
<img src="https://s16.postimg.org/uglrcnkkl/insert_new_key_binary_tree.png" width="50%" height="50%" />

## If you have a good hash function in almost cases you just filled new empty space in Address Table. But if you have much data, even with good hash function you will have much collisions. Here illustrated Best Case for Hashtable. 
<img src="https://s16.postimg.org/4wjh67z6t/insert_new_key_hashtable.png" width="50%" height="50%" />

## If you insert a new key into Trie, you needn't reallocate or balancing data and you can use existing segments as part of your new key.
<img src="https://s16.postimg.org/4ii56mf39/insert_new_key_trie.png" width="50%" height="50%" />

## If you search a key in Binary Tree you need always make long jumps. If you have 1 million keys these jumps will be about 20.
<img src="https://s16.postimg.org/j0fcem6ed/lookup_new_key_binary_tree.png" width="50%" height="50%" />

## In Hashtable in Best Case you put your key into hash function and after calculation of address you make one long jump by this address. But that's not enough, when you come by address you need scan your key again to ensure that this address is right. In collision case you need scan all scope of collided keys.
<img src="https://s16.postimg.org/nxswzq8dh/lookup_new_key_hashtable.png" width="50%" height="50%" />

## In Trie you always scan key only once. It could costs several long jumps, but maximum amount of jumps always constant. It like a maze - you need find right way.
<img src="https://s16.postimg.org/ua829kbfp/lookup_new_key_trie.png" width="50%" height="50%" />

## What happens if we want scan range of keys ?
<img src="https://s16.postimg.org/uygwsia5h/scan_range_from_to.png" width="50%" height="50%" />

## In Binary Tree first of all you need find first key and then scan next keys after him.
<img src="https://s16.postimg.org/63xeyfpb9/scan_range_binary_tree.png" width="50%" height="50%" />

## There is no good way scan a Hashtable because all data are unordered.
<img src="https://s16.postimg.org/vlfti171h/scan_range_hashtable.png" width="50%" height="50%" />

## In Trie you can scan only sub tree
<img src="https://s16.postimg.org/jv1w0ne91/scan_range_trie.png" width="50%" height="50%" />

## Of course the real life a little bit complex than illustrated simple cases above. But we hope you understand main idea of Trie.
















## Probably, this is most optimized Trie structure in the World ! Thats all what you need know about this project :)

**HArrayInt** - Key\Value In Memory structure for 32bit keys

**HArrayVarRAM** - Key\Value In Memory structure for keys with variety size

------------------

## Why we love Trie ? Because it has much more functionality and stability than Hashtables and much more faster than Binary Trees. Let's compare properties:

![alt tag](https://s16.postimg.org/6zis60mol/functionality.png)

------------------

## Result of Benchmarks

**Configuration**

| Param     | Value    |
| --------|---------|
| OS  | Ubuntu 16.1   |
| Processor | Intel(R) Core(TM) i5-6200U CPU @ 2.30 GHz 2.40 GHz |
| RAM | 8 GB |
| SSD | 256 GB |

![alt tag](https://s15.postimg.org/gzww2zhor/i_Core5.png)

**Note**: All results in tables below in milliseconds. In green color best results.
SEQUENCE cases always illustrate Worst Case for VyMa/Trie algorithm.

------------------

## PART 1, Size of key is 32 bits (4 bytes)

What is *SEQUENCE* key generation ?
<br>Keys such as: 0,1,2,3,4,5 etc.

<br>What is *RANDOM* key generation ?
<br>Keys such as: 33246, 878878,13241334,3987654633,67,342424242 etc.
<br>(used rand() function)

What is *PERIOD* key generation ?
<br>Keys such as: 0, 17, 289, 4913, 83521 ... N * 17 etc.

![alt tag](https://s16.postimg.org/j96eaew9h/insert_seq_32bits.png)

![alt tag](https://s16.postimg.org/fads5bx05/lookup_seq_32bits.png)

![alt tag](https://s16.postimg.org/3wmngdx3p/insert_rand_32bits.png)

![alt tag](https://s16.postimg.org/egwkyz1lh/lookup_rand_32bits.png)

![alt tag](https://s16.postimg.org/akenp8r85/insert_period_32bits.png)

![alt tag](https://s16.postimg.org/q3gp03owl/lookup_period_32bits.png)

------------------

## PART 2, Size of key is 128 bits (16 bytes)

What is *SEQUENCE* key generation ?
<br>Keys such as (one number in brackets = 4 bytes): 
<br>[0 0 0 1]
<br>[0 0 0 2]
<br>[0 0 0 3]
<br>etc.

What is *RANDOM* key generation ?
<br>Keys such as (one number in brackets = Unsigned Integer = 4 bytes):
<br>[33246 878878 13241334 3987654634]
<br>[468900044 222345566 789 2334555]
<br>[231 735353535 867980433 7664234]
<br>etc.

What is *PERIOD* key generation ?
<br>Keys such as (one number in brackets = Unsigned Integer = 4 bytes):
<br>[0 0 0 0]
<br>[17 17 17 17]
<br>[289 289 289 289]
<br>[4913 4913 4913 4913]
<br>etc.

![alt tag](https://s16.postimg.org/txa59968l/insert_seq_128bits.png)

![alt tag](https://s16.postimg.org/hg82zu0gl/lookup_seq_128bits.png)

![alt tag](https://s16.postimg.org/fbj4l09g5/insert_rand_128bits.png)

![alt tag](https://s16.postimg.org/o44omfjyt/lookup_rand_128bits.png)

![alt tag](https://s16.postimg.org/bnys17bv9/insert_period_128bits.png)

![alt tag](https://s16.postimg.org/r70tc29jp/lookup_period_128bits.png)

------------------

## PART 3, Size of key is 64 chars (64 bytes)

What is *SIMILAR* key generation ?
<br>Keys such as strings:
<br>[000000000000000000000000000000000000000000000000000000005u2iOpq]
<br>[00000000000000000000000000000000000000000000000000000000t92hUGs]
<br>[00000000000000000000000000000000000000000000000000000000MuiSf9l]
<br>etc.

What is *RANDOM* key generation ?
<br>Keys such as strings:
<br>[hd9sfdjj5JjsdfnbmituyUiegThsssOpklruYYwgdfshfj994gshspPReu2iOpq]
<br>[uKJkj12DkLSljd43djfjlLLss43kjks9sEOWPjfdjfkjJJHEYWQQfjsfdk2hUGs]
<br>[UDFdjjfsjhsjhdleE0E9j7sfL5MBNwMZZas22gwwrHHJhfsjsfsJqqJhfhsf95l]
<br>etc.

![alt tag](https://s16.postimg.org/bvr0bgc7p/insert_similar_64chars.png)

![alt tag](https://s16.postimg.org/gf7uapjh1/lookup_similar_64chars.png)

![alt tag](https://s16.postimg.org/ih3qb7s2d/insert_rand_64chars.png)

![alt tag](https://s16.postimg.org/fkgpaxm8l/lookup_rand_64chars.png)

------------------

**Code of benchmarks**
https://github.com/Bazist/HArray/blob/master/HArray/Main.cpp

**More result of benchmarks**:
http://wiki.pikosec.com/index.php?title=VymaDB:Benchmarks

------------------
Copyright(C) 2010-2016 Vyacheslav Makoveychuk (email: slv709@gmail.com, skype: vyacheslavm81)

------------------
## ENJOY
