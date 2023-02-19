// MIT License
//
// Copyright (c) 2023 Darius J Chuck
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef DJEVKO_H
#define DJEVKO_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#define ERR_END 1
#define ERR_CLOSE 2
#define ERR_TAG 4
#define ERR_LEN 8
#define ERR_KEY 16
#define ERR_PRE 32
#define ERR_SUF 64
#define ERR_DIGITS 128

#define OPENER '['
#define CLOSER ']'
#define QUOTER '`'
#define QUOTER_STR "`"

typedef size_t Index;
typedef size_t Size;
typedef int Bool;

typedef struct Slice {
  const char* str; 
  Size len;
} Slice;

typedef struct Djevko Djevko;

struct Djevko {
  // note: source could be removed
  const char* source; 
  Slice* prefixes; 
  // todo?: Slice* tags;
  Djevko* subs; 
  Size len; 
  Size cap;
  // note: null tag has tag.str = NULL
  Slice tag;
  Slice suffix; 
};

//
// internal
//

// todo: is the use of static here correct?
static Bool is_end_tag_found(const char* source, Index aa, Index ab, Index taga, Index tagb);
static inline Bool is_end_tag_found(const char* source, Index aa, Index ab, Index taga, Index tagb) {
  Index i = aa, j = taga;
  for (; j < tagb; ++i, ++j) {
    if (source[i] != source[j]) return 0;
  }
  for (; i < ab; ++i) {
    if (isspace(source[i]) == 0) return 0;
  }
  return 1;
}
static Size try_parse_len_prefix(const char* source, Index a, Index b);
static inline Size try_parse_len_prefix(const char* source, Index a, Index b) {
  Size ret = 0;
  Index i = a;
  while (i < b) {
    char c = source[i];
    if (c >= '0' && c <= '9') {
      ret *= 10;
      ret += c - '0';
    } else return 0;
    i += 1;
  }
  return ret;
}

//
// exports
//

Djevko* Djevko_new(const char* source);
inline Djevko* Djevko_new(const char* source) {
  Djevko* j = (Djevko*)malloc(sizeof *j);
  j->source = source;
  j->prefixes = (Slice*)NULL;
  // ?todo:
  // j->tags = (Slice*)NULL;
  j->subs = (Djevko*)NULL;
  j->len = 0;
  j->cap = 0;
  // ?
  j->tag = (Slice){0,0};
  // ?
  j->suffix = (Slice){0,0};
  return j;
}
// todo:
void Djevko_sub_delete(Djevko* j);
inline void Djevko_sub_delete(Djevko* j) {
  // free(j->source);
  free(j->prefixes);
  // ?todo:
  // j->tags
  for (Index i = 0; i < j->cap; ++i) {
    Djevko_sub_delete(&j->subs[i]);
  }
  free(j->subs);
}
void Djevko_delete(Djevko** jptr);
inline void Djevko_delete(Djevko** jptr) {
  Djevko* j = *jptr;
  Djevko_sub_delete(j);
  free(j);
  jptr = NULL;
}

char* Slice_to_str(Slice s);
inline char* Slice_to_str(Slice s) {
  Size len = s.len + 1;
  char* ret = (char*)malloc(len);
  strncpy(ret, s.str, len - 1);
  ret[len - 1] = 0;
  return ret;
}
Bool Slice_equals_str(Slice s, const char* str);
inline Bool Slice_equals_str(Slice s, const char* str) {
  Size len = strlen(str);
  if (s.len != len) return 0;
  for (Index i = 0; i < len; ++i) {
    if (s.str[i] != str[i]) return 0;
  }
  return 1;
}
Bool Slice_equals_key(Slice s, const char* str, char* f, Index i);
inline Bool Slice_equals_key(Slice s, const char* key, char* seen_keys, Index key_index) {
  if (Slice_equals_str(s, key)) {
    if (seen_keys[key_index]) {
      printf("Duplicate key: %s", key);
      exit(ERR_KEY);
    }
    seen_keys[key_index] = 1;
    return 1;
  }
  return 0;
}

char* Djevko_as_str(Djevko* j);
inline char* Djevko_as_str(Djevko* j) {
  // todo: nicer error
  if (j->len > 0) exit(1);
  return Slice_to_str(j->suffix);
}
Bool Djevko_as_bool(Djevko* j);
inline Bool Djevko_as_bool(Djevko* j) {
  // todo: nicer error
  if (j->len > 0) exit(1);
  Slice s = j->suffix;
  if (Slice_equals_str(s, "true")) return 1;
  if (Slice_equals_str(s, "false")) return 0;
  // todo: nicer error
  exit(1);
}
long Djevko_as_long(Djevko* j);
inline long Djevko_as_long(Djevko* j) {
  // todo: nicer error
  if (j->len > 0) exit(1);
  Slice s = j->suffix;
  char* endptr;
  long ret = strtol(s.str, &endptr, 0);
  // todo: nicer error
  if (endptr != s.str + s.len) exit(1);
  return ret;
}

void Djevko_check_prefix_empty(Djevko* j, Index i);
inline void Djevko_check_prefix_empty(Djevko* j, Index i) {
  Slice s = j->prefixes[i];
  if (Slice_equals_str(s, "") == 0) {
    printf("Expected prefix #%zu to be empty! Got: %s", i, Slice_to_str(s));
    exit(ERR_PRE);
  }
}
void Djevko_check_suffix_empty(Djevko* j);
inline void Djevko_check_suffix_empty(Djevko* j) {
  Slice s = j->suffix;
  if (Slice_equals_str(j->suffix, "") == 0) {
    printf("Expected suffix to be empty! Got: %s", Slice_to_str(s));
    exit(ERR_SUF);
  }
}

const char* Bool_as_str(Bool b);
inline const char* Bool_as_str(Bool b) {
  if (b == 0) return "false";
  return "true";
}

// todo:
void vardump(Djevko* j) {
  for (Index i = 0; i < j->len; ++i) {
    char* str = Slice_to_str(j->prefixes[i]);
    Size len = strlen(str);
    if (len > 0) printf("%zu'%s", len, str);

    printf("[");
    vardump(&j->subs[i]);
    printf("]");
  }
  char* str = Slice_to_str(j->suffix);
  Size len = strlen(str);
  if (len > 0) printf("%zu'%s", len, str);
}

Djevko* Djevko_parse_len(const char* str, size_t len);
inline Djevko* Djevko_parse_len(const char* str, size_t len) {
  size_t i = 0;
  size_t j = 0;

  // note: these could be registers
  size_t a = 0;
  size_t b = 0;
  size_t aposi = -1;

  Index taga, tagb;
  Djevko* parent = Djevko_new(str);

  size_t parents_capacity = 8;
  size_t parents_length = 0;
  Djevko** parents = (Djevko**)malloc(parents_capacity * sizeof(Djevko*));

  outer: while (i < len) {
    taga = 0;
    tagb = 0;

    // skip to first nonspace, set j to that
    while (1) {
      if (i >= len) break;
      char c = str[i];
      if (isspace(c) == 0) break;
      i += 1;
    }
    j = i;
    while (1) {
      if (i >= len) {
        parent->suffix = (Slice){str+j,i-j};
        goto end;
      }
      // todo: put in chr register
      char c = str[i];
      if (c == QUOTER) {
        Size len_prefix = try_parse_len_prefix(str, j, i);
        if (len_prefix > 0) {
          i += 1;
          a = i;
          i += len_prefix;
          b = i;
          c = str[i];
          if (c == OPENER) {
            goto open;
          }
          else if (c == CLOSER) {
            goto close;
          }
          else {
            // todo: nice error msgs
            printf("ERROR '%c' %zu", c, i);
            exit(ERR_LEN);
          }
        }

        taga = j; 
        tagb = i;
        i += 1;
        break;
      }
      else if (c == OPENER) {
        a = j;
        b = i;

        while (b > a && isspace(str[b - 1])) {
          b -= 1;
        }

        goto open;
      }
      else if (c == CLOSER) {
        a = j;
        b = i;

        while (b > a && isspace(str[b - 1])) {
          b -= 1;
        }

        goto close;
      }
      i += 1;
    }

    aposi = -1;
    j = i;

    while (1) {
      if (i >= len) {
        if (aposi != -1 && is_end_tag_found(str, aposi, i, taga, tagb)) {
          parent->tag = (Slice){str+taga, tagb-taga};
          parent->suffix = (Slice){str+j, aposi - 1 - j};
        } else {
          exit(ERR_TAG);
        }
        goto end;
      }
      char c = str[i];
      if (c == QUOTER) {
        aposi = i + 1;
      }
      else if (aposi != -1) {
        if (c == OPENER && is_end_tag_found(str, aposi, i, taga, tagb)) {
          a = j;
          b = aposi - 1;
          goto open;
        }
        else if (c == CLOSER && is_end_tag_found(str, aposi, i, taga, tagb)) {
          a = j;
          b = aposi - 1;
          goto close;
        }
      }
      i += 1;
    }
  }
  end:
  if (parents_length > 0) exit(ERR_END);

  free(parents);
  return parent;

  open: {
    // push subdjevko
    Size len = parent->len;
    Size cap = parent->cap;
    if (len == cap) {
      if (cap == 0) {
        cap = 1;
      } else {
        cap *= 2;
      }
      // realloc subs, realloc prefixes:
      parent->prefixes = (Slice*)realloc(parent->prefixes, cap * sizeof(Slice));
      parent->subs = (Djevko*)realloc(parent->subs, cap * sizeof(Djevko));

      // note: not exactly necessary
      memset(parent->prefixes + len, 0, (cap - len) * sizeof(Slice));
      memset(parent->subs + len, 0, (cap - len) * sizeof(Djevko));

      parent->cap = cap;
    }
    // ?
    parent->prefixes[len] = (Slice){str+a,b-a};
    parent->subs[len] = (Djevko){str, NULL, NULL, 0, 0, {0, 0}, {0, 0}};
    parent->len += 1;

    // push parent
    if (parents_length == parents_capacity) {
      parents_capacity *= 2;
      parents = (Djevko**)realloc(parents, parents_capacity * sizeof(Djevko*));
    }
    parents[parents_length] = parent;
    parents_length += 1;

    parent = &parent->subs[len];
    i += 1;
    goto outer;
  }

  close: {
    if (parents_length == 0) exit(ERR_CLOSE);

    parent->suffix = (Slice){str+a, b-a};

    // pop parent
    parents_length -= 1;
    parent = parents[parents_length];

    i += 1;
    goto outer;
  }
}

Djevko* Djevko_parse(const char* str);
inline Djevko* Djevko_parse(const char* str) {
  return Djevko_parse_len(str, strlen(str));
}

Size digit_count(Size n);
inline Size digit_count(Size n) {
  if (n < 10u) return 1;
  if (n < 100u) return 2;
  if (n < 1000u) return 3;
  if (n < 10000u) return 4;
  if (n < 100000u) return 5;
  if (n < 1000000u) return 6;
  if (n < 10000000u) return 7;
  if (n < 100000000u) return 8;
  if (n < 1000000000u) return 9;
  if (n < 10000000000u) return 10;
  if (n < 100000000000u) return 11;
  if (n < 1000000000000u) return 12;
  if (n < 10000000000000u) return 13;
  if (n < 100000000000000u) return 14;
  if (n < 1000000000000000u) return 15;
  if (n < 10000000000000000u) return 16;
  if (n < 100000000000000000u) return 17;
  if (n < 1000000000000000000u) return 18;
  if (n < 10000000000000000000u) return 19;

  // 18446744073709551615
  if (n <= UINT64_MAX) return 20;

  printf("Number %zu is too large to count its digits!", n);
  exit(ERR_DIGITS);
}

Slice escape_len(const char* str, Size len);
inline Slice escape_len(const char* str, Size len) {
  int maxeqc = -1;
  for (Index i = 0; i < len; ++i) {
    char c = str[i];
    if (maxeqc == -1) {
      if (c == OPENER || c == CLOSER) maxeqc = 0;
    }
    if (c == QUOTER) {
      int eqc = 1;
      Index j = i + 1;
      while (str[j] == '=') {
        ++eqc;
        ++j;
      }
      i = j - 1;
      if (eqc > 5) {
        // todo: extract to len_prefix(str, len)
        Size digits = digit_count(len);
        Size maxlen = digits + 1 + len + 1;
        char* ret = (char*)malloc(maxlen);
        snprintf(ret, maxlen, "%zu" QUOTER_STR "%s", digits, str);
        return (Slice){ret,maxlen-1};
      }
      if (eqc > maxeqc) maxeqc = eqc;
    }
  }
  if (maxeqc == -1) return (Slice){str,len};
  char* tag = (char*)malloc(maxeqc + 1);
  for (Index i = 0; i < maxeqc; ++i) {
    tag[i] = '=';
  }
  tag[maxeqc] = 0;
  Size maxlen = 2*maxeqc+2+len+1;
  char* ret = (char*)malloc(maxlen);

  snprintf(ret, maxlen, "%s" QUOTER_STR "%s" QUOTER_STR "%s", tag, str, tag);
  free(tag);
  return (Slice){ret,maxlen-1};
}
const char* escape(const char* str);
inline const char* escape(const char* str) {
  return escape_len(str, strlen(str)).str;
}

#endif