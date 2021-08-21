#include "cgi.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sqlite3.h>

static bool g_in_body = false;
static char *g_session_file = "../.session.db";
static sqlite3 *g_session = NULL;
static long g_session_id = 0;
static char *g_session_stored[32] = { 0 };
static char *g_session_keys[32] = { 0 };
static int g_session_nstored = 0;

#define SETUP_SESSION_SQL "CREATE TABLE IF NOT EXISTS sessions (" \
	"session_id INTEGER NOT NULL," \
	"key TEXT NOT NULL," \
	"value TEXT NOT NULL DEFAULT('')," \
	"PRIMARY KEY(session_id, key)" \
	");"

#define SESSION_BASE 36

#define SET_SESSION_SQL "INSERT INTO sessions (session_id, key, value) " \
	"VALUES(?, ?, ?) " \
	"ON CONFLICT(session_id, key) " \
	"DO UPDATE SET value = excluded.value;"

#define GET_SESSION_SQL "SELECT value FROM sessions WHERE session_id = ? AND key = ?;"

void body()
{
	if (!g_in_body)
	{
		puts("\r");
	}

	g_in_body = true;
}

void finish()
{
	printf("\n\n");
}

void header(char *format, ...)
{
	va_list list;
	va_start(list, format);

	vprintf(format, list);
	
	va_end(list);

	puts("\r");
}

void die(char *format, ...)
{
	body();

	printf("<h1 style=color:red;>");

	va_list list;
	va_start(list, format);

	vprintf(format, list);
	
	va_end(list);

	printf("</h1>\n");

	exit(1);
}

void set_cookie(char *name, char *value)
{
	header("Set-Cookie: %s=%s", name, value);
}

void set_cookie_int(char *name, long value)
{
	header("Set-Cookie: %s=%ld", name, value);
}

char *cookie(char *name)
{
	static bool init = false;
	static char *cookies[64];
	static int num_cookies = 0;

	if (!init)
	{
		char *cookie_header = getenv("HTTP_COOKIE");
		if (!cookie_header)
			cookie_header = "";

		for (char *tok = strtok(cookie_header, "; "); tok; tok = strtok(NULL, "; "))
		{
			cookies[num_cookies++] = tok;
		}
	}

	for (int i = 0; i < num_cookies; i++)
	{
		int len = strlen(name);

		if (strncmp(name, cookies[i], len) == 0)
		{
			return cookies[i] + len + 1;
		}
	}

	return NULL;
}

void session_file(const char *path)
{
	g_session_file = (char *)path;
}

void init_session()
{
	if (sqlite3_open(g_session_file, &g_session) != SQLITE_OK)
	{
		char buf[48];
		fprintf(stderr, "session file: %s/%s\n", getcwd(buf, 48), g_session_file);
		die("Could not open session file");
	}

	sqlite3_stmt *setup;
	if (sqlite3_prepare_v2(g_session, SETUP_SESSION_SQL, -1, &setup, NULL) == SQLITE_OK)
	{
		if (sqlite3_step(setup) != SQLITE_DONE)
		{
			sqlite3_finalize(setup);
			die("SQLite sessions statement failed");
		}
	}
	else
	{
		sqlite3_finalize(setup);
		die("Could not set up sessions SQLite DB");
	}

	sqlite3_finalize(setup);

	struct timeval now;
	gettimeofday(&now, NULL);

	srandom(now.tv_usec + now.tv_sec * 1000);
	g_session_id = random();
		
	char *session_cookie = cookie("SESSION");
	bool changed_session = true;

	if (session_cookie)
	{
		char *end;

		long id = strtol(session_cookie, &end, SESSION_BASE);

		fprintf(stderr, "session cookie = '%s', %ld\n", session_cookie, id);

		if (end == session_cookie + strlen(session_cookie))
		{
			g_session_id = id;
			changed_session = false;
		}
	}

	if (changed_session)
		set_cookie_int("SESSION", g_session_id);
}

void set_session(char *key, char *value)
{
	sqlite3_stmt *stmt;

	if (sqlite3_prepare_v2(g_session, SET_SESSION_SQL, -1, &stmt, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(stmt, 1, g_session_id);
		sqlite3_bind_text(stmt, 2, key, -1, NULL);
		sqlite3_bind_text(stmt, 3, value, -1, NULL);

		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			fprintf(stderr, "Failed to set session\n");
		}

		sqlite3_finalize(stmt);
	}
	else
	{
		die("Failed to set session");
	}

	for (int i = 0; i < g_session_nstored; i++)
	{
		if (strcmp(key, g_session_keys[i]) == 0)
		{
			g_session_stored[i] = strdup(value);
			return;
		}
	}

	g_session_keys[g_session_nstored] = strdup(key);
	g_session_stored[g_session_nstored++] = strdup(value);
}

char *session(char *key)
{
	for (int i = 0; i < g_session_nstored; i++)
	{
		if (strcmp(key, g_session_keys[i]) == 0)
			return g_session_stored[i];
	}
	
	char *value = NULL;

	sqlite3_stmt *stmt;

	if (sqlite3_prepare_v2(g_session, GET_SESSION_SQL, -1, &stmt, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(stmt, 1, g_session_id);
		sqlite3_bind_text(stmt, 2, key, -1, NULL);

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			value = strdup((char *)sqlite3_column_text(stmt, 0));

			g_session_keys[g_session_nstored] = strdup(key);
			g_session_stored[g_session_nstored++] = value;
		}
	}

	return value;
}

char *session_or(char *key, char *otherwise)
{
	char *res = session(key);

	if (!res)
	{
		set_session(key, otherwise);
		res = otherwise;
	}

	return res;
}

__attribute__((destructor)) static void cleanup_session_cache()
{
	for (int i = 0; i < g_session_nstored; i++)
	{
		free(g_session_stored[i]);
		free(g_session_keys[i]);
	}

	if (g_session)
	{
		sqlite3_close_v2(g_session);
	}
}
