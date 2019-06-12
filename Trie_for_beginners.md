# Trie for Beginners
___
# MEMORY
## Best way understand difference between associative arrays if you realise how several keys will be arranged inside. We have only three Keys - each with four segments (one segment could be 1 byte or 4 byte or 8 bytes - it doesn't matter in our explanation).
<img src="https://github.com/Bazist/HArray/blob/master/Images/keys.png" width="50%" height="50%" />

## In Binary Tree these keys will be arranged as ordered data
<img src="https://github.com/Bazist/HArray/blob/master/Images/inside_binary_tree.png?raw=true" width="50%" height="50%" />

## In Hashtable these keys will be arranged in more complex structure as unordered data. Also sometimes it needs more extra space.
<img src="https://github.com/Bazist/HArray/blob/master/Images/inside_hashtable.png?raw=true" width="50%" height="50%" />

## In Trie these keys will be arranged as ordered data and sometimes it requires a little bit less space.
<img src="https://github.com/Bazist/HArray/blob/master/Images/inside_trie.png?raw=true" width="50%" height="50%" />

___
# INSERT
## OK. What happens if we want insert a new key into each structure ?
<img src="https://github.com/Bazist/HArray/blob/master/Images/new_key.png?raw=true" width="50%" height="50%" />

## In Binary Tree we need find right place in ordered sequence and insert our key there. Sometimes we need split our page to do balancing of tree.
<img src="https://github.com/Bazist/HArray/blob/master/Images/insert_new_key_binary_tree.png?raw=true" width="50%" height="50%" />

## If you have a good hash function in almost cases you just filled new empty space in Address Table. But if you have much data, even with good hash function you will have much collisions. Here illustrated Best Case for Hashtable without any collisions. 
<img src="http://www.booben.com/insert_new_key_hashtable.png?raw=true" width="50%" height="50%" />

## If you insert a new key into Trie, you needn't reallocate or balancing data and you can use existing segments as part of your new key.
<img src="https://github.com/Bazist/HArray/blob/master/Images/insert_new_key_trie.png?raw=true" width="50%" height="50%" />

___
# LOOKUP
## If you search a key in Binary Tree you need always makes long jumps. If you have 1 million keys these jumps will be about 20.
<img src="https://github.com/Bazist/HArray/blob/master/Images/lookup_new_key_binary_tree.png?raw=true" width="50%" height="50%" />

## In Hashtable in Best Case you put your key into hash function and after calculation of address you make one long jump by this address. But this is not enough, when you come by address you need scan your key again to ensure that this address is right. In collision case you need scan all scope of collided keys.
<img src="https://github.com/Bazist/HArray/blob/master/Images/lookup_new_key_hashtable.png?raw=true" width="50%" height="50%" />

## In Trie you always scan key only once. It could costs several long jumps, but maximum amount of jumps always constant. It like a maze - you need find right way.
<img src="https://github.com/Bazist/HArray/blob/master/Images/lookup_new_key_trie.png?raw=true" width="50%" height="50%" />

___
# SCAN BY RANGE
## What happens if we want scan range of keys ?
<img src="https://github.com/Bazist/HArray/blob/master/Images/scan_range_from_to.png?raw=true" width="50%" height="50%" />

## In Binary Tree first of all you need find first key (from) and then scan next keys after.
<img src="https://github.com/Bazist/HArray/blob/master/Images/scan_range_binary_tree.png?raw=true" width="50%" height="50%" />

## There is no good way scan a Hashtable because all data unordered.
<img src="https://github.com/Bazist/HArray/blob/master/Images/scan_range_hashtable.png?raw=true" width="50%" height="50%" />

## In Trie you need scan only sub tree
<img src="https://github.com/Bazist/HArray/blob/master/Images/scan_range_trie.png?raw=true" width="50%" height="50%" />
___

## Of course the real life a little bit complex than illustrated simple cases above. And real implementations could be much more effective than mentioned here, but it was only for understand main idea of Trie.

___
## ENJOY
