#pragma once

#include <stdarg.h>

void header(char *format, ...);
void set_cookie(char *name, char *value);
void set_cookie_int(char *name, long value);
char *cookie(char *name);
void die(char *format, ...);
void body();
void finish();

void session_file(const char *path);
void set_session(char *key, char *value);
char *session(char *key);
char *session_or(char *key, char *otherwise);

void init_session();
