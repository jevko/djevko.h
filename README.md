[EXPERIMENTAL]

# djevko.h

A variant of Jevko implemented in C which differs from the original in that:

* it is defined over bytes rather than Unicode code points
* it uses [heredocs and length-prefixing](HEREDOCS.md) instead of digraphs to "escape" special characters
* it [automatically trims whitespace](WHITESPACE.md)

## [Rationale](RATIONALE.md)

## Features

* single-header library: simply `#include "djevko.h"` and use!
* provides utility functions to help with (de)serialization, escaping, checking, converting, etc.
* should be fairly fast (yet to be benchmarked)
* binary data can be mixed with text data
* human readable and writable
* super minimal and flexible

## Example

An example piece of data like:

```
name [Jon Wąpierz 😀]
age [32]
is cool [true]
fav colors [
  [red]
  [green]
  [blue]
]
```

can be parsed into C structs like:

```c
typedef struct {
  char** colors;
  Size len;
} FavColors;

typedef struct {
  char* name;
  int age;
  Bool is_cool;
  FavColors fav_colors;
} Person;
```

And the other way around.

## Usage

All you need is [djevko.h](djevko.h).

See [example.c](example.c).

It shows how to (de)serialize C structs/arrays/strings/numbers/booleans to/from Djevko.

You can compile and run the example with:

```
gcc example.c && ./a.out 
```
