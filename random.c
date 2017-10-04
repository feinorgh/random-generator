/*
 * random generates series of unique random integers in an arbitrary range
 *
 * Copyright (C) 2005, 2017  Pär Karlsson <feinorgh@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have recieved a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <search.h>
#include <errno.h>
#include <string.h>

gmp_randstate_t state;
int verbose = 0;
int userandom = 0;
size_t size_of_set = 1;
mpz_t range, low, high;
const char *version = "1.2";


void
print_version()
{
    printf("random version %s\n", version);
    printf("Copyright (C) 2005, 2017  Pär Karlsson <feinorgh@gmail.com>\n");
    printf("This is free software, and you are welcome to redistribute it "
           "under\ncertain conditions; please read the accompanying LICENSE "
           "file for details.\n");
}

void
print_usage() {
    char *usage_string = "Usage: random [-vVrh] [-v|--verbose] [-V|--version] [-l|--lower=<number>]\n"
                         "        [-u|--upper=<number>] [-c|--count=<number>] [-f|--file=<filename>]\n"
                         "        [-r|--random] [-?|--help] [--usage]\n";
    printf("%s", usage_string);
}

void
print_help() {
    char *help_string = "Usage: random [OPTION...]\n"
                        "  -v, --verbose             Be verbose (shows debug info)\n"
                        "  -V, --version             Show version and copyright information.\n"
                        "  -l, --lower=<number>      Lower bound (inclusive). Default is 1.\n"
                        "  -u, --upper=<number>      Upper bound (inclusive). Default is 100.\n"
                        "  -c, --count=<number>      Generate this many unique numbers.\n"
                        "  -f, --file=<filename>     Read parameters from this file.\n"
                        "  -r, --random              Use '/dev/random' instead of '/dev/urandom'\n"
                        "\n"
                        "Help options:\n"
                        "  -h, --help                Show this help message\n"
                        "      --usage               Display brief usage message\n";
    printf("%s", help_string);
}

void
cleanup() {
    mpz_clear(low);
    mpz_clear(high);
    mpz_clear(range);
}

void
clear_number(void *p)
{
    mpz_clear(*(mpz_t*)p);
}

void
treeaction(const void *nodep, const VISIT which, const int depth)
{
    mpz_t *datap;
    switch(which) {
    case preorder:
        break;
    case postorder:
        datap = *(mpz_t **)nodep;
        gmp_printf("%Zd\n", *(mpz_t *)datap);
        break;
    case endorder:
        break;
    case leaf:
        datap = *(mpz_t **)nodep;
        gmp_printf("%Zd\n", *(mpz_t *)datap);
        break;
    }
}

void
randomInteger(mpz_t result, const mpz_t low, const mpz_t high)
{
    mpz_sub(range, high, low);
    mpz_add_ui(range, range, 1);
    mpz_urandomm(result, state, range);
    mpz_add(result, result, low);
}

int
compare(const void *a, const void *b) {
    return(mpz_cmp(*(mpz_t *)a, *(mpz_t *)b));
}

void
generateSeries(size_t size_of_set, mpz_t low, mpz_t high)
{
    char *suffix = "s";
    if (verbose) {
        if (size_of_set < 2) {
            suffix = "";
        }
        printf("Generating %zu number%s between %s and %s...\n",
               size_of_set,
               suffix,
               mpz_get_str(NULL, 10, low),
               mpz_get_str(NULL, 10, high)
              );
    }

    mpz_t tmp;
    mpz_init(tmp);
    mpz_cdiv_q_ui(tmp, range, 2);
    if (mpz_cmp_ui(tmp, size_of_set) < 0) {
        /*
         * 1) Decide how many numbers we need to generate
         *    (range - size_of_set)
         * 2) Make a tree with those numbers
         * 3) Start from low, go to high, print all numbers that
         *    are not in the tree.
         * */
        mpz_sub_ui(tmp, range, size_of_set);
        size_t invsize = mpz_get_ui(tmp);
        mpz_t *number = malloc(sizeof(mpz_t) * invsize);
        void *root = NULL;
        size_t i = 0;
        for ( i = 0; i < invsize; i++ ) {
            mpz_init(number[i]);
            do {
                randomInteger(number[i], low, high);
            } while (tfind((void *)number[i], &root, compare) != NULL);
            tsearch((void *)number[i], &root, compare);
        }
        mpz_set(tmp, low);
        while (mpz_cmp(tmp, high)) {
            if (tfind((void *)tmp, &root, compare) == NULL) {
                /* printf("%s\n", mpz_get_str(NULL, 10, tmp)); */
                gmp_printf("%Zd\n", tmp);
            }
            mpz_add_ui(tmp, tmp, 1);
        }
        tdestroy(root, clear_number);
        if (number != NULL) {
            free(number);
        }
    } else {
        mpz_t *number = malloc(sizeof(mpz_t) * size_of_set);
        if (number == NULL) {
            fprintf(stderr, "%s\n", strerror(errno));
            exit(1);
        }
        void *root = NULL;
        size_t i = 0;
        for ( i = 0; i < size_of_set; i++ ) {
            mpz_init(number[i]);
            do {
                randomInteger(number[i], low, high);
            } while (tfind((void *)number[i], &root, compare) != NULL);
            tsearch((void *)number[i], &root, compare);
        }

        twalk(root, treeaction);
        tdestroy(root, clear_number);
        if (number != NULL) {
            free(number);
        }
    }
    mpz_clear(tmp);
}

int
initRandom(void)
{
    extern gmp_randstate_t state;
    gmp_randinit_default(state);
    mpz_t seed;
    mpz_init(seed);

    size_t count = 256; /* We read this many bytes from /dev/urandom */
    char *seedstart = malloc(count*sizeof(char*) + 1);

    char *filename;
    if (userandom) {
        filename = "/dev/random";
    } else {
        filename = "/dev/urandom";
    }

    int devrandom = open(filename, O_RDONLY);
    read(devrandom, seedstart, count);
    close(devrandom);
    mpz_import(seed, 1, 1, 1, 0, 0, seedstart);
    free(seedstart);

    gmp_randseed(state, seed);
    mpz_clear(seed);
    return 0;
}

int
read_params_from_file(const char *optarg) {
    if (verbose) {
        printf("Reading parameters from file '%s'", optarg);
    }
    char *buf1, *buf2;
    FILE *fp = fopen(optarg, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: %s\n", optarg, strerror(errno));
        return -1;
    }
    int fields_parsed = fscanf(fp, "%ms %ms\n", &buf1, &buf2);
    if (fields_parsed < 2) {
        fprintf(stderr, "Error: Could not parse contents of '%s'.\n",
                optarg);
        fprintf(stderr, "Got %d fields.\n", fields_parsed);
    } else {
        mpz_set_str(low, (char *)buf1, 10);
        mpz_set_str(high, (char *)buf2, 10);
    }
    fclose(fp);
    if (buf1) {
        free(buf1);
    }
    if (buf2) {
        free(buf2);
    }
    return 1;
}

int
parseOptions(int argc, char * const argv[])
{
    int c;
    mpz_t arg_count;
    mpz_init(arg_count);
    mpz_set_ui(arg_count, size_of_set);

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"verbose", no_argument,       0, 'V'},
            {"version", no_argument,       0, 'v'},
            {"lower",   required_argument, 0, 'l'},
            {"upper",   required_argument, 0, 'u'},
            {"count",   required_argument, 0, 'c'},
            {"file",    required_argument, 0, 'f'},
            {"random",  no_argument,       0, 'r'},
            {"help",    no_argument,       0, 'h'},
            {"usage",   no_argument,       0, 's'},
            {0,         0,                 0, 0}

        };
        c = getopt_long(argc, argv, "vVl:u:c:f:rhu", long_options, &option_index);
        if ( c == -1 ) {
            break;
        }

        switch (c) {
        case 0:
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;
        case 'h':
            print_help();
            cleanup();
            exit(0);
        case 's':
            print_usage();
            cleanup();
            exit(0);
        case 'v':
            print_version();
            cleanup();
            exit(0);
        case 'V':
            verbose = 1;
            break;
        case 'l':
            if (mpz_set_str(low, optarg, 10) == -1) {
                fprintf(stderr, "Error: Could not parse '%s' into a number.\n",
                        optarg);
                exit(1);
            }
            break;
        case 'u':
            if (mpz_set_str(high, optarg, 10) == -1) {
                fprintf(stderr, "Error: Could not parse '%s' into a number.\n",
                        optarg);
                exit(1);
            }
            break;
        case 'c':
            if (mpz_set_str(arg_count, optarg, 10) == -1) {
                fprintf(stderr, "Error: Could not parse '%s' into a number.\n",
                        optarg);
                exit(1);
            }
            if(mpz_fits_ulong_p(arg_count) == 0) {
                fprintf(stderr, "Error: Count is too large.\nMax value for "
                        "size of set is %lu.\n", ULONG_MAX);
                exit(1);
            }
            size_of_set = mpz_get_ui(arg_count);
            break;
        case 'f':
            if ( read_params_from_file(optarg) < 1 ) {
                mpz_clear(arg_count);
                cleanup();
                exit(1);
            }
            break;
        case 'r':
            userandom = 1;
        case '?':
            break;
        }
    }
    mpz_init(range);
    mpz_sub(range, high, low);
    if(mpz_cmp(arg_count, range) >= 0) {
        fprintf(stderr, "Error: Size given (%s) exceeds range (%s).\n"
                "No unique random numbers can be generated.\n",
                mpz_get_str(NULL, 10, arg_count),
                mpz_get_str(NULL, 10, range)
               );
        exit(1);
    }
    mpz_clear(arg_count);
    return 0;
}


int
main(int argc, const char **argv)
{
    mpz_init_set_str(low, "1", 10);
    mpz_init_set_str(high, "100", 10);
    parseOptions(argc, (char * const*)argv);
    initRandom();
    generateSeries(size_of_set, low, high);
    gmp_randclear(state);
    cleanup();
    return 0;
}

