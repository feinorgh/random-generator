# random

## PURPOSE

__random__ generates series of unique random integers in an arbitrary range.

More specifically, it generates a sorted set of unique integers by sampling
without replacement from a specified range.

It guarantees that no number occurs more than once in the output.

It makes use of the GMP Library (The GNU Multiple Precision Arithmetic
Library) for generating integers of any imaginable magnitude, given the
limitations of memory, processing power, the shell's ability to provide long
enough arguments, the user's ability to type them, et cetera.


## DEPENDENCIES

- [GMP — The GNU Multiple Precision Arithmetic Library](https://gmplib.org/)

On Debian/Ubuntu-based systems:

    sudo apt install libgmp-dev

On Red Hat/Fedora-based systems:

    sudo dnf install gmp-devel


## BUILDING

    make

Or, with additional warnings enabled:

    make CFLAGS="-Wall -Wextra -Wformat -g"


## OPERATION

By default the program generates one number between 1 and 100.

It accepts a number of options to control the size and range of the generated
set:

    -l, --lower=<number>     Lower bound (inclusive). Default is 1.
    -u, --upper=<number>     Upper bound (inclusive). Default is 100.
    -c, --count=<number>     Generate this many unique numbers. Default is 1.
    -f, --file=<filename>    Read lower and upper bounds from a file.
    -r, --random             Use '/dev/random' instead of '/dev/urandom'.
    -V, --verbose            Be verbose (shows debug info).
    -v, --version            Show version and copyright information.

The lower and upper bounds can accept negative numbers (although the lower
bound must always be lower than the upper).

The output numbers are printed in ascending order.

There is a limit on the size of the set (currently set to `ULONG_MAX` on your
system) which is primarily designed for simplicity of implementation.
Generating very large sets of random numbers is pretty silly anyway, and
takes a lot of time and memory. If you want to, you can try to generate a set
of 20,000,000 numbers, but don't come crying and say I didn't warn you when
your memory has run out.

There's nothing stopping you from shooting yourself in the foot. And there
shouldn't be.


## HUMOROUSLY BIG NUMBERS

You may find that you want to generate extremely big random numbers, and
that the argument list becomes too long for the system to handle. In that
case you can read values from a file with the `-f <file>` option. The file
shall contain only two values, separated by a space: the lower and the upper
bound. Again, these may be negative values. You can also read from stdin,
of course.


## ALGORITHM

Random numbers are seeded from `/dev/urandom` (or `/dev/random` with the
`-r` flag). The seed is 256 bytes of raw entropy imported into a GMP integer,
which is then used to initialise GMP's Mersenne Twister PRNG via
`gmp_randinit_default` and `gmp_randseed`.

Individual integers are drawn using `mpz_urandomm`, which produces a
uniformly distributed random integer in `[0, range)`, where
`range = upper − lower + 1`. This value is then shifted by `lower` to map it
into the requested range.

Uniqueness is enforced using a POSIX binary search tree (`tsearch`/`tfind`
from `<search.h>`). Each newly generated number is checked against the tree;
if a duplicate is found, a new number is drawn until a unique one is obtained.

When the requested count exceeds half the available range, the program uses a
**complementary sampling** approach: instead of generating the desired numbers
directly, it generates the *excluded* numbers (there are fewer of them), then
outputs every integer in the full range that does not appear in the exclusion
set. This keeps worst-case work proportional to the smaller of `count` and
`range − count`, a technique described in:

> Donald E. Knuth, *The Art of Computer Programming*, Volume 2:
> *Seminumerical Algorithms*, 3rd ed., Section 3.4.2, Algorithm S
> (Selection sampling).

The collected numbers are stored in a BST and printed via an in-order
traversal (`twalk`), which is why the output is always in ascending order.


## REFERENCES

- [GMP — The GNU Multiple Precision Arithmetic Library](https://gmplib.org/)
- [GMP Random Number Functions](https://gmplib.org/manual/Random-Number-Functions)
- [Mersenne Twister PRNG (Wikipedia)](https://en.wikipedia.org/wiki/Mersenne_Twister)
- [Fisher-Yates shuffle (Wikipedia)](https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle)
- [Reservoir sampling (Wikipedia)](https://en.wikipedia.org/wiki/Reservoir_sampling)
- [random(4) — Linux manual page](https://man7.org/linux/man-pages/man4/random.4.html)
- [tsearch(3) — Linux manual page](https://man7.org/linux/man-pages/man3/tsearch.3.html)


## EXCUSE

This was written mostly for learning the GNU MP library and for the fun of it.
Hopefully you will find this software useful and maybe even slightly fun.

I used its predecessor for generating image indexes in a shell script on a
web server once (it could only generate measly standard ints though).

And if you make a Lotto row with the help of this program, I wish you the best
of luck trying to win.

This program was first written in 2005. Updates regarding safe memory usage
and code quality were done in 2017.

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
a random seed (ddl)).

Pär Karlsson, September 2017
