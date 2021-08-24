session(3)

# NAME

session, set_session, session_or, session_file, init_session - query
and update the users session.

# SYNOPSIS

```
#include "cgi.h"

void session_file(const char *path);
void set_session(char *key, char *value);
char *session(char *key);
char *session_or(char *key, char *otherwise);

void init_session();
```

# DESCRIPTION

Any program that uses sessions should begin with calls to
*session_file*() and *init_session*(). The former sets the path to the
session database (which should point to the same file for all pages in
a web application). This is a SQLite 3 database that is created
automatically if it does not exist.

*init_session*() will read the "SESSION" cookie for the users session
ID if it is set, and will otherwise generate a new session ID and
update the cookie accordingly.

The *session*() function queries the user's session for a property
_key_. If that property is not set it returns _NULL_.

The *session_or*() function is similar, except instead of returning
_NULL_ it returns _otherwise_ if the property is not set.

The *set_session*() function sets a property _key_ to _value_ in the
current session.

# EXAMPLE

```
#include "cgi.h"

int main()
{
	session_file("../sessions.db");
	init_session();

	// The "name" property in the current session, or "Jim" if that
	// property is not set.
	char *name = session_or("name", "Jim");

	// Create a session if it does not exist, set "user-id" to "1234"
	set_session("user-id", "1234");

	// The "favorite-color" property, or NULL if it is not set
	char *favorite_color = session("favorite-color");
}
```

# RETURN VALUE

The return value of *session*() and *session_or*() must not be
freed. Treat it as a static string. Additionally, calling either of
those functions multiple times in a programs lifetime does not result
in repeated calls to the database. If the program requires session
information that might be changed externally during its lifetime, it
should use some other means of storing it.
