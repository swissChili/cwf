doc_sources = $(glob doc/*.scd)
doc_targets = $(patsubst %.scd,%,$(doc_sources))

public/main: main.c tl.c cgi.c
	$(CC) -o $@ $^ -g -lsqlite3

serve: public/main view/index.html
	cgiserver -p 8080 -f public/


docs: $(doc_targets)
doc/%: doc/%.scd
	scdoc < $< > $@
