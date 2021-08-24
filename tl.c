#include "tl.h"
#include "cgi.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static FILE *fnull;

__attribute__((constructor)) static void init()
{
	fnull = fopen("/dev/null", "w");
}

__attribute__((destructor)) static void close()
{
	fclose(fnull);
}

static int peek(FILE *in)
{
	int c = fgetc(in);
	ungetc(c, in);

	return c;
}

static void skipws(FILE *in)
{
	while (isspace(peek(in)))
		fgetc(in);
}

char *read_to(FILE *from, char until)
{
	char *heap = calloc(12, 1);
	int capacity = 12;
	int size = 0;

	int c;

	while ((c = peek(from)) != until)
	{
		if (c == EOF)
			die("EOF while parsing");

		fgetc(from);

		if (size == capacity)
		{
			capacity += 8;
			heap = realloc(heap, capacity);
		}

		heap[size++] = c;
	}

	fgetc(from);

	heap[size] = 0;
	return heap;
}

struct tl_val tl_gets(struct tl_val haystack, char *needle)
{
	if (haystack.type == TL_ENV)
	{
		for (struct tl_env *env = haystack.v_env; env; env = env->next)
		{
			if (strcmp(env->key, needle) == 0)
			{
				return env->val;
			}
		}

		goto failure;
	}
	else if (haystack.type == TL_LIST && strcmp(needle, "count") == 0)
	{
		return tl_int(haystack.v_list_len);
	}

failure:
	return (struct tl_val){ .type = TL_NULL };
}

struct tl_val tl_geti(struct tl_val haystack, int needle)
{
	if (haystack.type == TL_LIST)
	{
		int i = 0;

		for (struct tl_list *list = haystack.v_list; list; list = list->next, i++)
		{
			if (i == needle)
				return list->val;
		}

		goto failure;
	}

failure:
	return (struct tl_val){ .type = TL_NULL };
}

struct tl_val tl_expr(struct tl_val ctx, char *expr)
{
	struct tl_val current = ctx;
	char *save;

	for (char *tok = strtok_r(expr, ". ", &save); tok; tok = strtok_r(NULL, ". ", &save))
	{
		char *end;
		long val = strtol(tok, &end, 10);

		if (end != tok + strlen(tok))
		{
			current = tl_gets(current, tok);
		}
		else
		{
			current = tl_geti(current, (int)val);
		}
	}

	return current;
}

void tl_eval(FILE *from, FILE *to, struct tl_val ctx)
{
	for (int c = fgetc(from); c != EOF; c = fgetc(from))
	{
		if (c != '@')
		{
			fputc(c, to);
		}
		else
		{
			int n = peek(from);

			if (n == '@')
				fputc(fgetc(from), to);
			else if (n == '/')
			{
				// end
				fgetc(from);

				return;
			}
			else if (n == '{' || n == ':')
			{
				bool escape = n == '{';

				if (n == ':')
				{
					fgetc(from);
					if (peek(from) != '{')
						die("Expected { after @:");
				}
				
				fgetc(from);
				char *expr = read_to(from, '}');
				struct tl_val val = tl_expr(ctx, expr);
				free(expr);

				tl_write(to, val, escape);
			}
			else if (n == '?' || n == '!')
			{
				bool negate = n == '!';
				fgetc(from);

				if (peek(from) != '{')
					die("Expected { after @?");

				fgetc(from);
				char *expr = read_to(from, '}');
				struct tl_val val = tl_expr(ctx, expr);
				free(expr);

				bool bool_val = tl_to_bool(val);

				if (negate ? !bool_val : bool_val)
				{
					tl_eval(from, to, ctx);
				}
				else
				{
					tl_eval(from, fnull, ctx);
				}
			}
			else if (n == '*')
			{
				fgetc(from);

				if (peek(from) != '{')
					die("Expected { after @?");

				fgetc(from);
				char *expr = read_to(from, '}');
				struct tl_val val = tl_expr(ctx, expr);
				free(expr);

				if (val.type == TL_LIST)
				{
					int pos = ftell(from);

					for (struct tl_list *list = val.v_list; list; list = list->next)
					{
						tl_eval(from, to, list->val);
						
						if (list->next)
							fseek(from, pos, SEEK_SET);
					}
					if (!val.v_list)
					{
						tl_eval(from, fnull, ctx);
					}
				}
				else
				{
					die("Can only loop over list for now.");
				}
			}
		}
	}
}

struct tl_val tl_list()
{
	struct tl_val val =
		{
			.type = TL_LIST,
			.v_list = NULL,
			.v_list_end = NULL,
			.v_list_len = 0,
		};

	return val;
}

struct tl_val tl_env()
{
	struct tl_val val =
		{
			.type = TL_ENV,
			.v_env = NULL,
		};

	return val;
}

struct tl_val tl_string(char *string)
{
	struct tl_val val =
		{
			.type = TL_STR,
			.v_str = strdup(string),
		};

	return val;
}

struct tl_val tl_int(int i)
{
	struct tl_val val =
		{
			.type = TL_INT,
			.v_int = i,
		};

	return val;
}

struct tl_val tl_null()
{
	return (struct tl_val)
		{
			.type = TL_NULL,
		};
}

void tl_append(struct tl_val *list, struct tl_val value)
{
	if (list->type != TL_LIST)
		return;

	struct tl_list *node = malloc(sizeof(struct tl_list));
	node->next = NULL;
	node->val = value;

	if (list->v_list == NULL)
	{
		list->v_list = list->v_list_end = node;
	}
	else
	{
		list->v_list_end->next = node;
		list->v_list_end = node;
	}

	list->v_list_len++;
}

void tl_set(struct tl_val *env, char *key, struct tl_val value)
{
	if (env->type != TL_ENV)
		return;

	struct tl_env *node = malloc(sizeof(struct tl_env));
	node->next = NULL;
	node->key = strdup(key);
	node->val = value;

	if (env->v_env == NULL)
	{
		env->v_env = env->v_env_end = node;
	}
	else
	{
		env->v_env_end->next = node;
		env->v_env_end = node;
	}
}

void tl_free(struct tl_val val)
{
	if (val.type == TL_LIST)
	{
		for (struct tl_list *list = val.v_list, *next; list; list = next)
		{
			next = list->next;

			tl_free(list->val);
			free(list);
		}
	}
	else if (val.type == TL_ENV)
	{
		for (struct tl_env *env = val.v_env, *next; env; env = next)
		{
			next = env->next;

			tl_free(env->val);
			free(env->key);
			free(env);
		}
	}
	else if (val.type == TL_STR)
		free(val.v_str);
}

void write_html_escape(FILE *to, char *string, bool escape)
{
	if (escape)
	{
		for (; *string; string++)
		{
			char c = *string;

			if (c == '<')
				fprintf(to, "&lt;");
			else if (c == '&')
				fprintf(to, "&amp;");
			else if (c == '>')
				fprintf(to, "&gt;");
			else if (c == '"')
				fprintf(to, "&quot;");
			else
				fputc(c, to);
		}
	}
	else
	{
		fprintf(to, "%s", string);
	}
}

void tl_write(FILE *to, struct tl_val val, bool html_escape)
{
	if (val.type == TL_STR)
	{
		write_html_escape(to, val.v_str, html_escape);
	}
	else if (val.type == TL_INT)
	{
		fprintf(to, "%d", val.v_int);
	}
	else if (val.type == TL_LIST)
	{
		fprintf(to, "[ ");

		for (struct tl_list *list = val.v_list; list; list = list->next)
		{
			tl_write(to, list->val, html_escape);
			fprintf(to, ", ");
		}

		fprintf(to, "]");
	}
	else if (val.type == TL_ENV)
	{
		fprintf(to, "{ ");

		for (struct tl_env *env = val.v_env; env; env = env->next)
		{
			write_html_escape(to, env->key, html_escape);
			fprintf(to, " = ");
			tl_write(to, env->val, html_escape);
			fprintf(to, ", ");
		}
		
		fprintf(to, "}");
	}
	else
		fprintf(to, "NULL");
}

bool tl_to_bool(struct tl_val val)
{
	if (val.type == TL_INT)
		return !!val.v_int;
	else if (val.type == TL_LIST)
		return !!val.v_list_len;

	return true;
}
