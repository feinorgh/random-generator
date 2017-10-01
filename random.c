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
#include <popt.h>
#include <limits.h>
#include <search.h>
#include <errno.h>
#include <string.h>

gmp_randstate_t state;
int verbose = 0;
int show_version = 0;
int userandom = 0;
size_t size_of_set = 0;
mpz_t range, low, high;
const char *version = "1.1";

void
usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s", error, addl);
    exit(exitcode);
}

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
        printf("%s\n", mpz_get_str(NULL, 10, *(mpz_t *)datap));
        break;
    case endorder:
        break;
    case leaf:
        datap = *(mpz_t **)nodep;
        printf("%s\n", mpz_get_str(NULL, 10, *(mpz_t *)datap));
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
                printf("%s\n", mpz_get_str(NULL, 10, tmp));
            }
            mpz_add_ui(tmp, tmp, 1);
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
        if (number) {
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
parseOptions(int argc, const char **argv)
{
    char c;
    poptContext optCon;

    char *lo = NULL;
    char *hi = NULL;
    char *sz = NULL;
    char *paramfile = NULL;

    struct poptOption optionsTable[] = {
        {   "verbose", 'v', POPT_ARG_NONE, &verbose, 0,
            "Be verbose (shows debug info)", NULL
        },
        {   "version", 'V', POPT_ARG_NONE, &show_version, 0,
            "Show version and copyright information.", NULL
        },
        {   "lower", 'l', POPT_ARG_STRING, &lo, 0,
            "Lower bound (inclusive). Default is 1.", "<number>"
        },
        {   "upper", 'u', POPT_ARG_STRING, &hi, 0,
            "Upper bound (inclusive). Default is 100.", "<number>"
        },
        {   "count", 'c', POPT_ARG_STRING, &sz, 0,
            "Generate this many unique numbers.", "<number>"
        },
        {   "file", 'f', POPT_ARG_STRING, &paramfile, 0,
            "Read parameters from this file.", "<filename>"
        },
        {   "random", 'r', POPT_ARG_NONE, &userandom, 0,
            "Use '/dev/random' instead of '/dev/urandom'"
        },
        POPT_AUTOHELP
        { NULL, 0, 0, NULL, 0 }
    };

    optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);

    c = poptGetNextOpt(optCon);

    if (show_version) {
        print_version();
        exit(0);
    }

    if (lo) {
        if (mpz_set_str(low, lo, 10) == -1) {
            fprintf(stderr, "Error: Could not parse '%s' into a number.\n",
                    lo);
            exit(1);
        }
    }

    if (hi) {
        if (mpz_set_str(high, hi, 10) == -1) {
            fprintf(stderr, "Error: Could not parse '%s' into a number.\n",
                    hi);
            exit(1);
        }
    }

    char **buf1, **buf2;
    if (paramfile) {
        FILE *fp = fopen(paramfile, "r");
        if (fp == NULL) {
            fprintf(stderr, "%s: %s\n", paramfile, strerror(errno));
            exit(1);
        }
        int fields_parsed = gmp_fscanf(fp, "%as %as", &buf1, &buf2);
        if (fields_parsed < 2) {
            fprintf(stderr, "Error: Could not parse contents of '%s'.\n",
                    paramfile);
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
    }

    if (mpz_cmp(high, low) < 1) {
        fprintf(stderr, "Error: invalid range, lower (%s) >= upper (%s).\n",
                mpz_get_str(NULL, 10, low),
                mpz_get_str(NULL, 10, high));
        exit(1);
    }

    mpz_t argsz;
    mpz_init(argsz);
    mpz_init(range);
    mpz_sub(range, high, low);
    if (sz) {
        if (mpz_set_str(argsz, sz, 10) == -1) {
            fprintf(stderr, "Error: Could not parse '%s' into a number.\n",
                    sz);
            exit(1);
        }
        if (mpz_fits_ulong_p(argsz) == 0) {
            fprintf(stderr, "Error: Size given is too large.\nMax value for "
                    "size of set is %lu.\n", ULONG_MAX);
            exit(1);
        }
        if (mpz_cmp(argsz, range) >= 0) {
            fprintf(stderr, "Error: Size given (%s) exceeds range (%s).\n"
                    "No unique random numbers can be generated.\n",
                    mpz_get_str(NULL, 10, argsz),
                    mpz_get_str(NULL, 10, range)
                   );
            exit(1);
        }
        size_of_set = mpz_get_ui(argsz);
    }
    mpz_clear(argsz);

    if ( c < -1 ) {
        fprintf(stderr, "%s: %s\n",
                poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
                poptStrerror(c));
        return 1;
    }

    poptFreeContext(optCon);
    return 0;
}

int
main(int argc, const char **argv)
{
    mpz_init_set_str(low, "1", 10);
    mpz_init_set_str(high, "100", 10);
    size_of_set = 1;

    parseOptions(argc, argv);
    initRandom();
    generateSeries(size_of_set, low, high);

    mpz_clear(low);
    mpz_clear(high);
    gmp_randclear(state);
    return 0;
}

