#include "cgi.h"
#include "tl.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

struct {
	char *name;
	char *occupation;
} friends[] =
{
	{ "Jim", "Lawn Mower Salesman" },
	{ "Richard", "Raisin Enthusiast" },
	{ "Jenny", "Harp enjoyer"}
};
int nfriends = 3;

int main(int argc, char **argv, char **env)
{
	header("Status: 200 OK");
	header("Content-Type: text/html");

	session_file("../.session.db");
	init_session();

	body();

	char *name = session_or("name", "Richard");

	FILE *index = fopen("../view/index.html", "r");

	struct tl_val ctx = tl_env();
	tl_set(&ctx, "name", tl_string(name));
	tl_set(&ctx, "age", tl_int(23));

	struct tl_val f = tl_list();

	for (int i = 0; i < nfriends; i++)
	{
		struct tl_val friend = tl_env();
		tl_set(&friend, "name", tl_string(friends[i].name));
		tl_set(&friend, "occupation", tl_string(friends[i].occupation));

		tl_append(&f, friend);
	}

	tl_set(&ctx, "friends", f);

	tl_eval(index, stdout, ctx);

	tl_free(ctx);
	fclose(index);

	finish();
}
