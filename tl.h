#pragma once

#include <stdio.h>
#include <stdbool.h>

struct tl_val
{
	enum
	{
		TL_STR,
		TL_INT,
		TL_LIST,
		TL_ENV,
		TL_NULL
	} type;

	union
	{
		char *v_str;
		int v_int;
		struct tl_list *v_list;
		struct tl_env *v_env;
	};

	union
	{
		struct tl_list *v_list_end;
		struct tl_env *v_env_end;
	};

	int v_list_len;
};

struct tl_list
{
	struct tl_val val;
	struct tl_list *next;
};

struct tl_env
{
	char *key;
	struct tl_val val;
	struct tl_env *next;
};

struct tl_val tl_gets(struct tl_val haystack, char *needle);
struct tl_val tl_geti(struct tl_val haystack, int needle);

bool tl_to_bool(struct tl_val val);
void tl_write(FILE *to, struct tl_val val);
void tl_eval(FILE *from, FILE *to, struct tl_val ctx);

struct tl_val tl_expr(struct tl_val ctx, char *expr);

struct tl_val tl_list();
struct tl_val tl_env();
struct tl_val tl_string(char *string);
struct tl_val tl_int(int val);

void tl_append(struct tl_val *list, struct tl_val value);
void tl_set(struct tl_val *env, char *key, struct tl_val value);

void tl_free(struct tl_val val);
