# random

## PURPOSE

__random__ generates series of unique random integers in an arbitrary range.

More specifically it means that it generates an ordered set of unique integers.

It guarantees that no number occurs more than once.

It makes use of the GMP Library (The GNU MP Bignum Library) for generating
integers of any imaginable order,given the limitations on memory, processing
power, the shell's ability to provide long enough arguments, the user's
ability to type them et cetera.


## OPERATION

By default the program generates one number between 1 and 100.

It can take a number of options to control the size and range of the generated
set:

    -l, --lower=<number>     Lower bound (inclusive). Default is 1.
    -u, --upper=<number>     Upper bound (inclusive). Default is 100.
    -c, --count=<number>     Generate this many unique numbers.

The lower and upper bounds can accept negative numbers (although the lower
bound must always be lower than the upper).

There is a limit on the size of the set (currently set to `UINT_MAX` from your
system's `/usr/include/limits.h`) which is primarily designed for simplicity
of implementation. Generating very large sets of random numbers is pretty
silly anyway, and takes a lot of time and memory. If you want to, you
can try to generate a set of 20 000 000 numbers, but don't come
crying and say I didn't warn you when your memory has run out.

There's nothing stopping you from shooting yourself in the foot. And there
shouldn't be.


## HUMOROUSLY BIG NUMBERS

You may find that you want to generate extremely big random numbers, and
that the argument list becomes too long for the system to handle. In that
case you can read values from a file with the -f <file> option. The file
shall contain only two values, separated by a space: the lower and the upper
bound. Again, these may be negative values. You can also read from stdin,
of course.


## EXCUSE

This was written mostly for learning the GNU MP library and for the fun of it.
Hopefully you will find this software useful and a maybe even slightly fun.

I used it's predecessor for generating image indexes in a shell script on a
web server once. (It could only generate measly standard ints though).

And if you make a Lotto row with the help of this program I wish you the best
of luck trying to win.

This program was first written in 2005. Updates regarding safe memory usage
and updates to the code was done in 2017.

## DISCLAIMER

This software may contain bugs. If you find one, please notify me. Also, if
you have suggestions, improvements or comments, I'd like to hear about it.

Also, the software lacks test cases and QA, so you might not want to rely on
this particular piece of software to drive trains, planes or automobiles.


## THANK YOU

The good people writing and maintaining the GMP library.

The GNU people and Donald Knuth for the binary tree implementation used
herein.

Mikael Magnusson (Mikachu) and Erik Waling (ddl) for suggestions and
improvements (generate excluded numbers when the size of the set is more than
half the range (Mikachu); reading parameters from a file (Mikachu); reading
a random seed (ddl) ).

Pat Karlsson, September 2017
