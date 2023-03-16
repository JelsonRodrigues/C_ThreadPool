# ThreadPool done in C
A C ThreadPool library for linux implementation (not fully done yet) using pthread library

# Build & Run
There is a `Makefile` for building the project <br>
```shell
git clone https://github.com/JelsonRodrigues/C_ThreadPool
cd C_ThreadPool`
make release
```
The result is the executable in the `./bin` folder and all object files in the `./build` folder

# Features
## Vector Library
A vector library that can hold any data type and reallocate itself as it needs, with a lot of utility functions
## Producer Consumer Library
A producer consumer library that can handle buffers with any ammount of producers/consumers and when there is no more producers/consumers the consumers/producers get signaled to finish the execution. So a consumer dont wait forever for an item if there is no producers in the buffer.
All items that were produced will be consumed before consumers dropping the channel.
## Thread Pool Library
A thread pool library that allow the creation of an arbitrary number of threads using the `phtreads` library. The threads communicate with a producer consumer buffer and keep waiting for `Task` to arrive. These `Task` can be inserted into a buffer, and the pool will execute the tasks.

# TODO
<input type="checkbox"> Finish the TheadPool implementation <br>
<input type="checkbox"> Test ProducerConsumer and TheadPool for deadlocks <br>
<input type="checkbox"> Test ProducerConsumer <br>
<input type="checkbox"> Document the code <br>
<input type="checkbox" checked> Test Vector for memory leaks and wrong acces <br>