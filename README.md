[EXPERIMENTAL]

# djevko.h

A variant of Jevko implemented in C which differs from the original in that:

* it is defined over bytes rather than Unicode code points
* it uses heredocs and length-prefixing instead of digraphs to "escape" special characters
* it automatically trims unquoted strings
* it uses `'` instead of `` ` `` as the third (next to `[` and `]`) special character (this may change)

## Features

* single-header library
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

See [example.c](example.c).

It shows how to (de)serialize C structs/arrays/strings/numbers/booleans to/from Djevko