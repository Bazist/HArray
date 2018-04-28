## Result of Benchmarks

**Configuration**

| Param     | Value    |
| --------|---------|
| OS  | Ubuntu 16.1   |
| Processor | Intel(R) Core(TM) i5-6200U CPU @ 2.30 GHz 2.40 GHz |
| RAM | 8 GB |
| SSD | 256 GB |

![alt tag](http://www.booben.com/i_Core5.png)

**Notes**: All results in tables below in milliseconds. In green color best results. In each benchmark Value size is 4 bytes (emulates a memory pointer of Value object). SEQUENCE cases always illustrate Worst Case for VyMa/Trie algorithm.

------------------

## PART 1, Size of key is 32 bits (4 bytes)

What is *SEQUENCE* key generation ?
<br>Keys such as: 0,1,2,3,4,5 etc.

<br>What is *RANDOM* key generation ?
<br>Keys such as: 33246, 878878,13241334,3987654633,67,342424242 etc.
<br>(used rand() function)

What is *PERIOD* key generation ?
<br>Keys such as: 0, 17, 289, 4913, 83521 ... N * 17 etc.

![alt tag](http://www.booben.com/insert_seq_32bits.png)

![alt tag](http://www.booben.com/lookup_seq_32bits.png)

![alt tag](http://www.booben.com/insert_rand_32bits.png)

![alt tag](http://www.booben.com/lookup_rand_32bits.png)

![alt tag](http://www.booben.com/insert_period_32bits.png)

![alt tag](http://www.booben.com/lookup_period_32bits.png)

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

![alt tag](http://www.booben.com/insert_seq_128bits.png)

![alt tag](http://www.booben.com/lookup_seq_128bits.png)

![alt tag](http://www.booben.com/insert_rand_128bits.png)

![alt tag](http://www.booben.com/lookup_rand_128bits.png)

![alt tag](http://www.booben.com/insert_period_128bits.png)

![alt tag](http://www.booben.com/lookup_period_128bits.png)

------------------

## PART 3, Size of key is 64 chars (64 bytes)

What is *SIMILAR* key generation ?
<br>Keys such as strings:
<br>[0000000000000000000000000000000000000000000000000000000t5u2iOpq]
<br>[0000000000000000000000000000000000000000000000000000000lt92hUGs]
<br>[0000000000000000000000000000000000000000000000000000000JMuiSf9l]
<br>etc.

What is *RANDOM* key generation ?
<br>Keys such as strings:
<br>[hd9sfdjj5JjsdfnbmituyUiegThsssOpklruYYwgdfshfj994gshspPReu2iOpq]
<br>[uKJkj12DkLSljd43djfjlLLss43kjks9sEOWPjfdjfkjJJHEYWQQfjsfdk2hUGs]
<br>[UDFdjjfsjhsjhdleE0E9j7sfL5MBNwMZZas22gwwrHHJhfsjsfsJqqJhfhsf95l]
<br>etc.

![alt tag](http://www.booben.com/insert_similar_64chars.png)

![alt tag](http://www.booben.com/lookup_similar_64chars.png)

![alt tag](http://www.booben.com/insert_rand_64chars.png)

![alt tag](http://www.booben.com/lookup_rand_64chars.png)

------------------

**Code of benchmarks**
https://github.com/Bazist/HArray/blob/master/HArray/Main.cpp

**More results of benchmarks**:
http://wiki.pikosec.com/index.php?title=VymaDB:Benchmarks

------------------
Copyright(C) 2010-2016 Vyacheslav Makoveychuk (email: slv709@gmail.com, skype: vyacheslavm81)

------------------
## ENJOY
