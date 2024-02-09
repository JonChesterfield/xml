#ifndef LIST_IMPL_H_INCLUDED
#define LIST_IMPL_H_INCLUDED

#ifndef LIST_H_INCLUDED
#error "Only intended for use as part of list.h"
#endif

#ifndef LIST_ATTRIBUTE
#error "LIST_ATTRIBUTE"
#endif

LIST_ATTRIBUTE void list_impl_memory_free(void* p)
{
  free(p); // can pass NULL to free
}

LIST_ATTRIBUTE void* list_impl_memory_alloc(size_t N)
{
  if (N == 0) { return NULL; }

  void* r = malloc(N);
  if (!r)
    {
      fprintf(stderr, "Malloc failed on %zu\n", N);
      exit(1);
    }
  assert(r);
  return r;
}

LIST_ATTRIBUTE void list_impl_set_element_type(list l, size_t index,
                                               enum list_element_type tag)
{
  assert(l.tag == list_element_type_list);
  assert(index < list_size(l));
  l.elts[index].tag = tag;
}

LIST_ATTRIBUTE list list_impl_peek_element_list(list l, size_t index)
{
  assert(l.tag == list_element_type_list);   
    assert(index < list_size(l));
  return l.elts[index];
}

#endif
