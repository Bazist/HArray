## Probably, this is most optimized Trie structure in the World ! Thats all what you need know about this project :)

**HArrayInt** - Key\Value In Memory structure for 32bit keys

**HArrayVarRAM** - Key\Value In Memory structure for keys with variety size

------------------

## Why we love Trie ? Because it has much more functionality and stability than Hashtables and much more faster that Binary Trees. Let's compare properties:

![alt tag](https://s16.postimg.org/6zis60mol/functionality.png)

------------------

## Result of Benchmarks

**Configuration**

| Param     | Value    |
| --------|---------|
| OS  | Ubuntu 16.1   |
| Processor | iCore 5 6400U |
| RAM | 8 GB |
| SSD | 256 GB |

![alt tag](https://s15.postimg.org/gzww2zhor/i_Core5.png)

------------------

## PART 1, Size of key is 32 bits (4 bytes)

What is *SEQUENCE* key generation ?
Keys such as: 0,1,2,3,4,5 etc.

What is *RANDOM* key generation ?
Keys such as: 33246, 878878,13241334,3987654633,67,342424242 etc.
(used rand() function)

What is *PERIOD* key generation ?
Keys such as: 0, 17, 289, 4913, 83521 ... N * 17 etc.

![alt tag](https://s16.postimg.org/j96eaew9h/insert_seq_32bits.png)

![alt tag](https://s16.postimg.org/fads5bx05/lookup_seq_32bits.png)

![alt tag](https://s16.postimg.org/3wmngdx3p/insert_rand_32bits.png)

![alt tag](https://s16.postimg.org/egwkyz1lh/lookup_rand_32bits.png)

![alt tag](https://s16.postimg.org/akenp8r85/insert_period_32bits.png)

![alt tag](https://s16.postimg.org/q3gp03owl/lookup_period_32bits.png)

------------------

## PART 2, Size of key is 128 bits (16 bytes)

What is *SEQUENCE* key generation ?
Keys such as (one number in brackets = 4 bytes): 
[0 0 0 1]
[0 0 0 2]
[0 0 0 3]
etc.

What is *RANDOM* key generation ?
Keys such as (one number in brackets = Unsigned Integer = 4 bytes): 
[33246 878878 13241334 3987654634]
[468900044 222345566 789 2334555]
[231 735353535 867980433 7664234]
etc.

What is *PERIOD* key generation ?
Keys such as (one number in brackets = Unsigned Integer = 4 bytes): 
[0 0 0 0]
[17 17 17 17]
[289 289 289 289]
[4913 4913 4913 4913]
etc.

![alt tag](https://s16.postimg.org/txa59968l/insert_seq_128bits.png)

![alt tag](https://s16.postimg.org/hg82zu0gl/lookup_seq_128bits.png)

![alt tag](https://s16.postimg.org/fbj4l09g5/insert_rand_128bits.png)

![alt tag](https://s16.postimg.org/o44omfjyt/lookup_rand_128bits.png)

![alt tag](https://s16.postimg.org/bnys17bv9/insert_period_128bits.png)

![alt tag](https://s16.postimg.org/r70tc29jp/lookup_period_128bits.png)

------------------

## PART 3, Size of key is 64 chars (64 bytes)

What is *SIMILAR* key generation ?
Keys such as strings:
[000000000000000000000000000000000000000000000000000000005u2iOpq]
[00000000000000000000000000000000000000000000000000000000t92hUGs]
[00000000000000000000000000000000000000000000000000000000MuiSf9l]
etc.

What is *RANDOM* key generation ?
Keys such as strings:
[hd9sfdjj5JjsdfnbmituyUiegThsssOpklruYYwgdfshfj994gshspPReu2iOpq]
[uKJkj12DkLSljd43djfjlLLss43kjks9sEOWPjfdjfkjJJHEYWQQfjsfdk2hUGs]
[UDFdjjfsjhsjhdleE0E9j7sfL5MBNwMZZas22gwwrHHJhfsjsfsJqqJhfhsf95l]
etc.

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

## ENJOY
