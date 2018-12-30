## Deep Learning Framework for NLP from scratch


#### Description
Significantly improved the speed of the highly optimized sequential program (based on B+ tree) using Pthread API and Parallel programming. 

Minimized overhead by reducing Mutex Lock (less shared memory by all processes), memory access (1D long array instead of 3D map), and using buffering (resolving dependency problem).

#### How to run the code
 - move to working directory.
$make : make object files (sugar.o queue.o lcgrand.o) and executable file named "sugar"

 - take test_setup.txt as input. Then generate sugar_map.txt & gen_people_list.txt as output.
 - take sugar_map.txt & gen_people_list.txt as input this time. Then generate output_map_table.txt & output_people_list.txt as output.
 - also display total running time of sugar program on shell.
 - you can specify the number of threads as an argument to the sugar program.
 - the minimum number of threads is 4 and the maximum number of threads is 96.
 - if you don't specify the number of threads, then the default number of thread is 8.
 - this code is based on pthread API.
$./sugar : run executable file (sugar)


$make clean: delete object files (sugar.o queue.o lcgrand.o) and files for debug (sugar.gch queue.gch lcgrand.gch).
