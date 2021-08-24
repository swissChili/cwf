tl_list(3)

# NAME

tl_list, tl_env, tl_string, tl_int - create a *struct tl_val* of a
certain type.

# SYNOPSIS

```
#include "tl.h"

struct tl_val tl_list();
struct tl_val tl_env();
struct tl_val tl_string(char *string);
struct tl_val tl_int(int val);
struct tl_val tl_null();
```

# DESCRIPTION

The functions *tl_list*(), *tl_env*(), *tl_string*(), and *tl_int*()
create and return a *struct tl_val* of the specified type. These
functions may allocate memory. The result of these functions should be
freed with *tl_free*() once they are done with.

# SEE ALSO

*tl_free*(3), *tl_append*(3), *tl_set*(3)
