# Dnipro Server

**DniproDB** one of the fastest document oriented database in the world. The database has innovated core based on **Trie** structures, **Inverted indexes** and **Lock Free** algorithms. We reinvented standard approaches in core to store **JSONs** only as set of keys.

# Why Dnipro ?
## REALLY EASY SYNTAX

Syntax so important for daily using.

Just put your bigest hierarchical **Business Object** to database in one row

```C#
DniproDB db = new DniproDB("localhost", 4477);

db.AddDoc(obj);
```
And that's all ! We indexed the object with all attributes and saved it in convinient **JSON** format. 

**Link queries** helps build complex queries

```C#
User[] users = 
db.GetWhere("{'FirstName':'John'})")
  .AndWhere("{'Address':{'Country':'UK'}})"
	  .Select<User>("{'FirstName':$,'LastName':$}");
```

You have opportunities to load and update your object partially with minimum traffic. You can do searches by documents and do searches inside one document.

And, of course, perfect **JOIN** ...
```C#
JoinResult[] jrs =
db.GetWhere("{'FirstName':'John'}")
  .Join("{'CarModel':$}","{'Type':'Car', 'Model':$}")
  .Join("{'Company':$}","{'Type':'Company', 'Name':$}")
  .Select<JoinResult>
         ("{'FirstName':$}{'Engine':$}{'City':$}");
```
Few clicks by mouse and database is ready for using in your environment 
(Ready for **Windows**/**Linux** platform and **.NET**, **Java** languages)

**Learn more** ? Just download our comprehensive [Dnipro In Use](http://booben.com/DniproDB_In_Use_EN.PDF) book.

## SO FAST

Speed, speed and again speed.

Do you remember ? **JSONs** it is just keys in embeded **Key\Value storage**.
So 
* When you insert a document, you just insert keys to Key\Value storage.
* When you insert new or update existing attribute in document,
  you just insert or update key in the storage without any overwriting of entire document.
* When you load part of document, you just lookup several keys. You don't need download all document on client at all.

And ... what you should know ... one operation of insert/update/lookup of long key costs about **fifty nanoseconds** on core level.
In regular benchmarks **Dnipro** faster than standard databases in [ten](http://forum.pikosec.com/viewforum.php?f=7) times and more.

Run in console **db.SelfTest()** to see how many seconds need to your computer to finish more than ten millions queries in our benchmark.

## RELIABLE DATABASE

Without any compromises.

**ACID** and three types of **Transactions** (ReadCommited, RepeatableRead, Snapshot). In case of need you can rollback of database on particular date using transaction log.

Very carefully blocking on **attribute level** gives maximum parallelism. Many transactions can change one document at the same time. Transactions wouldn’t overwrite changes to each other if their changed attributes are not crossed.

**Resolve any deadlocks** with full information in log: who, when, what documents, what attributes

And now it's **Opensource**.

## ENJOY
