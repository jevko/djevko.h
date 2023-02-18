#ifndef DJEVKO_H
#define DJEVKO_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define ERR_END 1
#define ERR_CLOSE 2
#define ERR_TAG 4
#define ERR_LEN 8
#define ERR_KEY 16
#define ERR_PRE 32
#define ERR_SUF 64

typedef size_t Index;
typedef size_t Size;
typedef int Bool;

typedef struct Slice {
  char* str; 
  Size len;
} Slice;

typedef struct Djevko Djevko;

struct Djevko {
  // note: source could be removed
  char* source; 
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
static Bool sliceEq(char* source, Index aa, Index ab, Index taga, Index tagb);
static inline Bool sliceEq(char* source, Index aa, Index ab, Index taga, Index tagb) {
  Index i = aa, j = taga;
  for (; j < tagb; ++i, ++j) {
    if (source[i] != source[j]) return 0;
  }
  for (; i < ab; ++i) {
    if (isspace(source[i]) == 0) return 0;
  }
  return 1;
}
static Size sliceToLen(char* source, Index a, Index b);
static inline Size sliceToLen(char* source, Index a, Index b) {
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

Djevko* Djevko_new(char* source);
inline Djevko* Djevko_new(char* source) {
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
Bool Slice_equals_str(Slice s, char* str);
inline Bool Slice_equals_str(Slice s, char* str) {
  Size len = strlen(str);
  if (s.len != len) return 0;
  for (Index i = 0; i < len; ++i) {
    if (s.str[i] != str[i]) return 0;
  }
  return 1;
}
Bool Slice_equals_key(Slice s, char* str, char* f, Index i);
inline Bool Slice_equals_key(Slice s, char* key, char* seen_keys, Index key_index) {
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
  char* str = Djevko_as_str(j);
  if (strcmp(str, "true") == 0) return 1;
  if (strcmp(str, "false") == 0) return 0;
  // todo: nicer error
  exit(1);
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

Djevko* Djevko_parse_len(char* str, size_t len);
inline Djevko* Djevko_parse_len(char* str, size_t len) {
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
      if (c == '\'') {
        Size len_prefix = sliceToLen(str, j, i);
        if (len_prefix > 0) {
          i += 1;
          a = i;
          i += len_prefix;
          b = i;
          c = str[i];
          if (c == '[') {
            goto open;
          }
          else if (c == ']') {
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
      else if (c == '[') {
        a = j;
        b = i;

        while (b > a && isspace(str[b - 1])) {
          b -= 1;
        }

        goto open;
      }
      else if (c == ']') {
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
        // todo: .trim() or .trimEnd()
        if (aposi != -1 && sliceEq(str, aposi, i, taga, tagb)) {
          parent->tag = (Slice){str+taga, tagb-taga};
          parent->suffix = (Slice){str+j, aposi - 1 - j}; //str.slice(j, aposi - 1)
        } else {
          exit(ERR_TAG);
        }
        goto end;
      }
      char c = str[i];
      if (c == '\'') {
        aposi = i + 1;
      }
      else if (aposi != -1) {
        if (c == '[' && sliceEq(str, aposi, i, taga, tagb)) {
          a = j;
          b = aposi - 1;
          goto open;
        }
        else if (c == ']' && sliceEq(str, aposi, i, taga, tagb)) {
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

Djevko* Djevko_parse(char* str);
inline Djevko* Djevko_parse(char* str) {
  return Djevko_parse_len(str, strlen(str));
}

// todo:
Size digit_count(Size n) {
  if (n < 10) return 1;
  if (n < 100) return 2;
  if (n < 1000) return 3;
  if (n < 10000) return 4;
  if (n < 100000) return 5;
  if (n < 1000000) return 6;
  if (n < 10000000) return 7;
  if (n < 100000000) return 8;
  if (n < 1000000000) return 9;
  if (n < 10000000000) return 10;
  if (n < 100000000000) return 11;
  if (n < 1000000000000) return 12;
  if (n < 10000000000000) return 13;

  // todo:
  exit(1000);
}
// todo: return Slice, accept len + versions which wrap/unwrap args/ret
char* escape(char* str) {
  int maxeqc = -1;
  Size len = strlen(str);
  for (Index i = 0; i < len; ++i) {
    char c = str[i];
    if (maxeqc == -1) {
      if (c == '[' || c == ']') maxeqc = 0;
    }
    if (c == '\'') {
      int eqc = 1;
      Index j = i + 1;
      while (str[j] == '=') {
        ++eqc;
        ++j;
      }
      i = j - 1;
      if (eqc > 5) {
        Size digits = digit_count(len);
        Size maxlen = digits + 1 + len + 1;
        char* ret = (char*)malloc(maxlen);
        snprintf(ret, maxlen, "%zu'%s", digits, str);
        return ret;
      }
      if (eqc > maxeqc) maxeqc = eqc;
    }
  }
  if (maxeqc == -1) return str;
  char* tag = (char*)malloc(maxeqc + 1);
  for (Index i = 0; i < maxeqc; ++i) {
    tag[i] = '=';
  }
  tag[maxeqc] = 0;
  Size maxlen = 2*maxeqc+2+len+1;
  char* ret = (char*)malloc(maxlen);

  snprintf(ret, maxlen, "%s'%s'%s", tag, str, tag);
  return ret;
}

#endif