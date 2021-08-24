#include "tl.h"
#include <sqlite3.h>

struct tl_val tl_sqlite_column(sqlite3_stmt *stmt, int column_index)
{
	int type = sqlite3_column_type(stmt, column_index);

	switch (type)
	{
	case SQLITE_INTEGER:
		return tl_int(sqlite3_column_int(stmt, column_index));
	case SQLITE_TEXT:
		return tl_string((char *)sqlite3_column_text(stmt, column_index));
	case SQLITE_NULL:
	default:
		return tl_null();
	}
}

struct tl_val tl_sqlite_row(sqlite3_stmt *stmt)
{
	int num_cols = sqlite3_column_count(stmt);

	struct tl_val val = tl_env();

	for (int i = 0; i < num_cols; i++)
	{
		const char *name = sqlite3_column_name(stmt, i);
		struct tl_val column = tl_sqlite_column(stmt, i);

		tl_set(&val, (char *)name, column);
	}

	return val;
}

struct tl_val tl_sqlite_all_rows(sqlite3_stmt *stmt)
{
	struct tl_val list = tl_list();

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		struct tl_val row = tl_sqlite_row(stmt);
		tl_append(&list, row);
	}

	return list;
}
