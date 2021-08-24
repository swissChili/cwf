tl_eval(3)

# NAME

tl_eval - evaluate a template in a certain context and write it to an
output stream

# SYNOPSIS

```
#include "tl.h"

void tl_eval(FILE *from, FILE *to, struct tl_val ctx);
```

# DESCRIPTION

*tl_eval*() reads the file _from_, evaluates it in the context of
_ctx_ according to *tl_html*(7), and writes the result to _to_.

# SEE ALSO

*tl_html*(7)
