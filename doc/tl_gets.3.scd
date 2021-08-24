tl_gets(3)

# NAME

tl_gets, tl_geti - extract a value from a *struct tl_val*.

# SYNOPSIS

```
#include "tl.h"

struct tl_val tl_gets(struct tl_val haystack, char *needle);
struct tl_val tl_geti(struct tl_val haystack, int needle);
```

# DESCRIPTION

If _haystack_ is a _TL_ENV_, *tl_gets*() returns the value in that
environment corresponding to the key _needle_.

If _haystack_ is a _TL_LIST_, *tl_gets*() returns its length if
_needle_ is "count", or _TL_NULL_ otherwise. *tl_geti*() returns the
item at index _needle_, starting from 0, or _TL_NULL_ if the index is
out of range.

In cases other than those specified above, both functions return
_TL_NULL_.
