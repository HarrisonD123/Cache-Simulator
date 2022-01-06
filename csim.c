#define _GNU_SOURCE
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "cachelab.h"

struct cacheline {
    char valid;
    unsigned long long tag;
    unsigned int timestamp; //timestamp to implement LRU eviction method
};

struct line {
    char type;
    long long unsigned int addr;
};

enum behavior {HIT, MISS, EVICT, HIT_HIT, MISS_HIT, EVICT_HIT};

typedef struct cacheline cacheline;
typedef struct line line_items;
typedef enum behavior behavior;

/*scans the input, parses out the memory access type and the memory address
  into the line struct*/
line_items parse_line(char* line) {
    line_items li;
    li.type = 0;
    li.addr = 0;
    sscanf(line, " %c %llx,", &li.type, &li.addr);
    return li;
}

/*malloc and initialize multiD array of cache lines
  Nummer of rows and columns based off of number of set bits and associativity*/
cacheline** init_cache(unsigned int s, unsigned int E) {
    cacheline** cache = (cacheline**)malloc((1 << s) * sizeof(cacheline*));
    for(int i = 0; i < (1 << s); i++) {
        cache[i] = (cacheline*)malloc(E * sizeof(cacheline));
        for(int j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].timestamp = E - 1 - j;
        }
    }
    return cache;
}

/*Create a mask for any number of set bits*/
long long unsigned int set_mask(unsigned int s) {
    return (1 << s) - 1;
}

/*Extracts set bits from address using mask*/
unsigned int get_set(long long unsigned int addr, long long unsigned int mask,
                     unsigned int b) {
    unsigned int set = (addr >> b) & mask;
    return set;
}

/*Extracts tag bits from address by shifting*/
long long unsigned int get_tag(long long unsigned int addr, unsigned int b,
                               unsigned int s) {
    return (addr >> b) >> s;
}

/*Update the timestamps for cache lines after mem access to keep
  track of LRU format*/
void update_timestamp(cacheline* c, unsigned int just_used, unsigned int E) {
    for(int i = 0; i < E; i++) {
        if(c[i].timestamp < just_used) {
            c[i].timestamp += 1;
        }
        else if(c[i].timestamp == just_used) {
            c[i].timestamp = 0;
        }
    }
}

/*Replace the tag for the most recently accessed block on a miss/evict*/
void replace(cacheline* c, long long unsigned int tag, unsigned int E) {
    for(int i = 0; i < E; i++) {
        if(c[i].timestamp == 0) {
            c[i].tag = tag;
            c[i].valid = 1;
        }
    }
}

/*Simulate the behavior of a cache memory access and return the behavior of the access
  (hit, miss, evict) as an enum*/
behavior access(cacheline* c, long long unsigned int tag,
                unsigned int E, char type) {
    behavior result = EVICT;
    unsigned int just_used = E - 1;

    for(int i = 0; i < E; i++) {
        if(c[i].valid == 1) {
            if(c[i].tag == tag) {
                result = HIT;
                just_used = c[i].timestamp;
                break;
            }
        }
        else {
            result = MISS;
            break;
        }
    }

    update_timestamp(c, just_used, E);
    
    if((result == MISS) || (result == EVICT)) {
        replace(c, tag, E);
    }


    if(type == 'M') {
        result += 3;
    }

    return result;
}

/*Updates the hit, miss, evict counts based on the behavior enum passed
  from main*/
void count_behavior(behavior result, int* h, int* m, int* e) {
    switch(result) {
        case HIT:
            *h += 1;
            break;
        case MISS:
            *m += 1;
            break;
        case EVICT:
            *m += 1;
            *e += 1;
            break;
        case HIT_HIT:
            *h += 2;
            break;
        case MISS_HIT:
            *h += 1;
            *m += 1;
            break;
        case EVICT_HIT:
            *h += 1;
            *m += 1;
            *e += 1;
            break;
    }
}

/*Free the multiD array of cache lines at the end of the trace file*/
void free_cache(cacheline** c, unsigned int s) {
    for(int i = 0; i < (1 << s); i++) {
        free(c[i]);
    }
    free(c);
}

int main(int argc, char *argv[]) {
    int c;
    unsigned int s = 0;
    unsigned int E = 0;
    unsigned int b = 0;
    char *tracefile;

    while ((c = getopt(argc, argv, ":s:E:b:t:")) != -1) {
        switch(c) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                break;
            case ':':
                fprintf(stderr,
                           "Option -%c requires an operand\n", optopt);
                break;
            case '?':
                fprintf(stderr,
                           "Unrecognized option: '-%c'\n", optopt);
                break;
        }
    }

    if(tracefile == NULL) {
        fprintf(stderr, "requires a file\n");
        exit(1);
    }

    cacheline** cache = init_cache(s, E);
    unsigned int set = 0;
    long long unsigned int tag = 0;

    line_items li;
    li.type = 0;
    li.addr = 0;

    char *string;
    size_t string_size = 0;
    FILE *fp = fopen(tracefile, "r");

    behavior result;

    int hits = 0;
    int misses = 0;
    int evictions = 0;

    while(getline(&string, &string_size, fp) != -1) {
        if(string[0] == 'I') {
            continue;
        }
        li = parse_line(string);
        set = get_set(li.addr, set_mask(s), b);
        tag = get_tag(li.addr, b, s);
        result = access(cache[set], tag, E, li.type);

        count_behavior(result, &hits, &misses, &evictions);
    }

    fclose(fp);
    free_cache(cache, s);

    printSummary(hits, misses, evictions);

    return 0;
}
