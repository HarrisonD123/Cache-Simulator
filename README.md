# Cache Simulator #

## Task ##
Simulate the behavior of a cache and report the number of hits, misses, and evictions given the cache's specifications

#### Features ####
1. Takes the cache's specifications (number of set bits, the number of block bits, the associativity) and the directory of a file that contains memory accesses (trace files) from the command line
2. Trace files are contained in the p3cache/traces directory
    1. L represents a data load, S represents a data store, and M represents a data modify. L and S can both either result in a hit, a miss, or an eviction. M can result in either __two hits__, __a miss and a hit__, or a __miss, an evict, and a hit__.
    2. The second argument in each line is represents a __32-bit__ memory address.
3. Reports the number of hits, misses, and evictions from each trace file.

## How to Run Locally ##
- Run this command `git clone https://github.com/HarrisonD123/Cache-Simulator.git`
- In the the project directory, run this command in the terminal: `make`
- To run the program, enter `./csim` in the terminal along with the number of set bits (`-s`), set associativity (`-E`), number of block bits (`-b`), and the trace file directory (`-t`):
    - A cache has 2^s sets and a 2^b block size
    - The last b bits of the given memory address are the block bits, and the s bits before them are the set bits. The rest of the bits at the front are the tag bits.
    - EX: `./csim -s 3 -E 2 -b 4 -t dave.trace`
- To see test cases for each trace file, run `./csim-test`
