tl_sqlite(3)

# NAME

tl_sqlite_row, tl_sqlite_column, tl_sqlite_all_rows - helper functions
for extracting data from SQLite queries.

# SYNOPSIS

```
#include "tl.h"

struct tl_val tl_sqlite_row(sqlite3_stmt *stmt);
struct tl_val tl_sqlite_column(sqlite3_stmt *stmt, int column_index);
struct tl_val tl_sqlite_all_rows(sqlite3_stmt *stmt);
```

# DESCRIPTION

The function *tl_sqlite_column*() converts the _column\_index_'th
column of the SQLite statement _stmt_ to a _struct tl\_val_. If the
column cannot be converted (if, for example, it is of an unsupported
type), _TL\_NULL_ is returned instead.

*tl_sqlite_row*() does this for every column in a row, returning a
_TL_ENV_ containing all the columns with the names they were given in
the SQLite *SELECT* statement.

*tl_sqlite_all_rows*() applies *tl_sqlite_row*() for every row in the
prepared statement _stmt_. This should be called after all parameters
have been bound in the statement, but before any calls to
*sqlite3_step*() have been made.
