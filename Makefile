doc_sources = $(wildcard doc/*.scd)
doc_targets = $(patsubst %.scd,%,$(doc_sources))

public/main: main.c cgi.c tl.c tl_sqlite.c
	$(CC) -o $@ $^ -g -lsqlite3

serve: public/main view/index.html
	cgiserver -p 8080 -f public/

docs: $(doc_targets)
doc/%: doc/%.scd
	scdoc < $< > $@
