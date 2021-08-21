cookie(3)

# NAME

cookie, set_cookie, set_cookie_int - get/set a browser cookie

# SYNOPSIS

```
#include "cgi.h"

void set_cookie(char *name, char *value);
void set_cookie_int(char *name, long value);
char *cookie(char *name);
```

# DESCRIPTION

The *cookie*() function returns the value of the browser cookie
_value_ in the current CGI context, or _NULL_ if the cookie is not
set.

The *set_cookie*() and *set_cookie_int*() functions set a cookie
_name_ to _value_.

# SEE ALSO

*session*(3), *set_session*(3), *session_or*(3)
