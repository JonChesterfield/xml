#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include "token.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct list;

enum list_element_type
{
  list_element_type_uninit = 0,
  list_element_type_token = 1,
  list_element_type_list = 2,
  list_element_type_out_of_memory = 3,
};

struct list
{
  enum list_element_type tag;
  union
  {
    token t;
    struct
    {
      const char* name;
      size_t N;
      struct list* elts;
    };
  };
};

typedef struct list list;

static_assert(sizeof(token) == 24, "");
static_assert(sizeof(list) == 32, "");

#define LIST_ATTRIBUTE __attribute__((unused)) static

LIST_ATTRIBUTE const char* list_name(list l)
{
  switch (l.tag)
    {
    case list_element_type_token:
      return l.t.name;
    case list_element_type_list:
      return l.name;
    case list_element_type_out_of_memory:
      return "out_of_memory";
    default:
      return "error";
    case list_element_type_uninit:
      return "uninit";
    }
}

LIST_ATTRIBUTE bool list_is_uninit(list l)
{
  return l.tag == list_element_type_uninit;
}
LIST_ATTRIBUTE bool list_is_out_of_memory(list l)
{
  return l.tag == list_element_type_out_of_memory;
}
LIST_ATTRIBUTE bool list_is_token(list l)
{
  return l.tag == list_element_type_token;
}
LIST_ATTRIBUTE bool list_is_list(list l)
{
  return l.tag == list_element_type_list;
}


LIST_ATTRIBUTE size_t list_size(list l)
{
  assert(list_is_list(l));
  return l.N;
}

LIST_ATTRIBUTE bool list_empty(list l) {  assert(list_is_list(l));
  return list_size(l) == 0; }


LIST_ATTRIBUTE enum list_element_type list_get_element_type(list l,
                                                            size_t index)
{
  assert(list_is_list(l));
  assert(index < list_size(l));
  return l.elts[index].tag;
}

LIST_ATTRIBUTE list list_uninit(void)
{
  list r = (list){.tag = list_element_type_uninit};
  assert(list_is_uninit(r));
  return r;
}

LIST_ATTRIBUTE list list_out_of_memory(void)
{
  list r = (list){.tag = list_element_type_out_of_memory};
  assert(list_is_out_of_memory(r));
  return r;
}

LIST_ATTRIBUTE list list_from_token(token s)
{
  // printf("Make list from token %s\n", s.name);
  list r = (list){.tag = list_element_type_token, .t = s};
  assert(list_is_token(r));
  return r;
}

LIST_ATTRIBUTE token list_token_to_token(list l) {
 assert(list_is_token(l));
 return l.t;
}

#include "list_impl.h"

LIST_ATTRIBUTE list list_make_uninitialised(const char* name, size_t N);
LIST_ATTRIBUTE void list_free(list l);

// Uses the name in the list as the element name
LIST_ATTRIBUTE void list_as_xml(FILE*, list l);

// Non-mutating compare
LIST_ATTRIBUTE bool list_equal(list x, list y);

// Deep copy, expensive style. Elements aren't reference counted.
LIST_ATTRIBUTE list list_clone(list l);

// Get token doesn't change the list as token is a value type
LIST_ATTRIBUTE token list_get_element_token(list l, size_t index);

// Write to an uninit or token slot
LIST_ATTRIBUTE void list_set_element_token(list l, size_t index, token tok);

// Leaves the corresponding entry undef, i.e. moves the element out
LIST_ATTRIBUTE list list_get_element_list(list l, size_t index);

// Write to an uninit slot
LIST_ATTRIBUTE void list_set_element_list(list l, size_t index, list sublist);

LIST_ATTRIBUTE list list_make_uninitialised(const char* name, size_t N)
{
  // printf("Make list %s with space %zu\n", name, N);
  list tmp = {.tag = list_element_type_out_of_memory};
  void* m = list_impl_memory_alloc(N * sizeof(struct list));
  if ((N == 0) || (m != NULL))
    {
      tmp.tag = list_element_type_list;
      tmp.name = name;
      tmp.N = N;
      tmp.elts = (list*)m;
      for (size_t i = 0; i < N; i++)
        {
          tmp.elts[i] = list_uninit();
        }
      assert(list_is_list(tmp));
    }
  return tmp;
}

LIST_ATTRIBUTE void list_free(list l)
{
  size_t N = list_size(l);
  if (l.tag == list_element_type_list)
    {
      for (size_t i = 0; i < N; i++)
        {
          switch (list_get_element_type(l, i))
            {
              case list_element_type_list:
                {
                  list_free(l.elts[i]);
                  break;
                }
              case list_element_type_uninit:
              case list_element_type_out_of_memory:
              case list_element_type_token:
              default:
                break;
            }
        }
      list_impl_memory_free(l.elts);
    }
  // otherwise no memory allocated
}

// This interface hasn't survived the data representation change

// Doesn't change the list as token is a value type
LIST_ATTRIBUTE token list_get_element_token(list l, size_t index)
{
  assert(l.tag == list_element_type_list);
  assert(index < list_size(l));
  assert(list_get_element_type(l, index) == list_element_type_token);
  return l.elts[index].t;
}

LIST_ATTRIBUTE void list_set_element_token(list l, size_t index, token tok)
{
  assert(l.tag == list_element_type_list);
  assert(index < list_size(l));
  assert(list_get_element_type(l, index) == list_element_type_uninit ||
         list_get_element_type(l, index) == list_element_type_token);
  l.elts[index].t = tok;
  list_impl_set_element_type(l, index, list_element_type_token);
}

// Leaves the corresponding entry undef, i.e. moves the element out
LIST_ATTRIBUTE list list_get_element_list(list l, size_t index)
{
  assert(l.tag == list_element_type_list);
  assert(list_get_element_type(l, index) == list_element_type_list);
  list r = list_impl_peek_element_list(l, index);
  list_impl_set_element_type(l, index, list_element_type_uninit);
  l.elts[index] = list_uninit();
  return r;
}

LIST_ATTRIBUTE void list_set_element_list(list l, size_t index, list sublist)
{
  assert(l.tag == list_element_type_list);
  assert(index < list_size(l));
  assert(list_get_element_type(l, index) == list_element_type_uninit);
  l.elts[index] = sublist;
  list_impl_set_element_type(l, index, list_element_type_list);
}

LIST_ATTRIBUTE bool list_equal(list x, list y)
{
  enum list_element_type tag = x.tag;
  if (tag != y.tag)
    {
      return false;
    }

  switch (tag)
    {
        // uninit and out of memory are atoms
      case list_element_type_uninit:
      case list_element_type_out_of_memory:
        return true;
      default:
        assert(!"equal");
        return false;
      case list_element_type_token:
        return token_equal(x.t, y.t);
      case list_element_type_list:
        break;
    }

  if (strcmp(x.name, y.name) != 0)
    {
      return false;
    }
  size_t N = x.N;
  if (N != y.N)
    {
      return false;
    }
  for (size_t i = 0; i < N; i++)
    {
      if (!list_equal(x.elts[i], y.elts[i]))
        {
          return false;
        }
    }
  return true;
}

LIST_ATTRIBUTE list list_clone(list src)
{
  enum list_element_type tag = src.tag;

  switch (tag)
    {
        // uninit and out of memory are atoms
      default:
        assert(!"clone");
        return list_uninit();
      case list_element_type_uninit:
        return list_uninit();
      case list_element_type_out_of_memory:
        return list_out_of_memory();
      case list_element_type_token:
        return list_from_token(src.t);
      case list_element_type_list:
        break;
    }

  size_t N = list_size(src);
  list dst = list_make_uninitialised(list_name(src), N);
  for (size_t i = 0; i < N; i++)
    {
      dst.elts[i] = list_clone(src.elts[i]);
    }
  assert(list_equal(src, dst));
  return dst;
}

LIST_ATTRIBUTE void list_as_xml(FILE* f, list l)
{
  switch(l.tag)
    {
    case list_element_type_uninit:
      {
        fprintf(f, "<Uninit/>");
        return;
      }
    case list_element_type_out_of_memory:
      {
        fprintf(f, "<OOM/>");
        return;
      }
    case list_element_type_token:
      {
        token_as_xml(f, list_token_to_token(l));
        return;
      }
    case list_element_type_list: break;
    }

  assert(list_is_list(l));
  size_t N = list_size(l);
  
  fprintf(f, "<%s>", list_name(l));
  for (size_t i = 0; i < N; i++)
    {
      list_as_xml(f, list_impl_peek_element_list(l, i));
    }
  fprintf(f, "</%s>", list_name(l));
}

#endif
