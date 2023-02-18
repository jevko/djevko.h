#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "djevko.h"

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

// converting a Djevko into an array
FavColors Djevko_as_fav_colors(Djevko* j) {
  Djevko_check_suffix_empty(j);

  Size len = j->len;
  char** colors = (char**)malloc(len * sizeof(char*));

  for (Index i = 0; i < len; ++i) {
    Djevko_check_prefix_empty(j, i);
    colors[i] = Djevko_as_str(&j->subs[i]);
  }

  return (FavColors){colors,len};
}

// converting a Djevko into a struct
Person Djevko_as_Person(Djevko* j) {
  Djevko_check_suffix_empty(j);

  Person p = {0,0,0,0};
  
  char seen_keys[4] = {0};
  for (Index i = 0; i < j->len; ++i) {
    Slice s = j->prefixes[i];

    if (Slice_equals_key(s, "name", seen_keys, 0)) {
      // converting a Djevko into a string
      p.name = Djevko_as_str(&j->subs[i]);
    } else if (Slice_equals_key(s, "age", seen_keys, 1)) {
      // converting a Djevko into an integer
      p.age = Djevko_as_long(&j->subs[i]);
    } else if (Slice_equals_key(s, "is cool", seen_keys, 2)) {
      // converting a Djevko into a boolean
      p.is_cool = Djevko_as_bool(&j->subs[i]);
    } else if (Slice_equals_key(s, "fav colors", seen_keys, 3)) {
      // converting a Djevko into a custom value
      p.fav_colors = Djevko_as_fav_colors(&j->subs[i]);
    }
  }

  return p;
}

Person Person_parse(char* str) {
  Djevko* j = Djevko_parse(str);
  Person p = Djevko_as_Person(j);
  Djevko_delete(&j);
  return p;
}

void Person_free(Person p) {
  free(p.name);
  for (Index i = 0; i < p.fav_colors.len; ++i) {
    free(p.fav_colors.colors[i]);
  }
  free(p.fav_colors.colors);
}

void Person_print(Person p, FILE* f) {
  fprintf(f, "name [%s]\nage [%d]\nis cool [%s]\nfav colors [\n", escape(p.name), p.age, Bool_as_str(p.is_cool));
  for (Index i = 0; i < p.fav_colors.len; ++i) {
    fprintf(f, "  [%s]\n", p.fav_colors.colors[i]);
  }
  fprintf(f, "]\n");
}

int main() {
  char* str = "name [  Jon Wąpierz 😀  ] age [32] is cool [true] fav colors [[red][green][blue]]   ";
  Person p = Person_parse(str);

  Person_print(p, stdout);

  Person_free(p);

  return 0;
}