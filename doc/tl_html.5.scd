tl_html(5)

# NAME

tl_html - HTML template format

# DESCRIPTION

tl_html is a format for writing dynamic HTML pages. A tl_html template
may mix data passed in from an outside source with HTML written
verbatim.

# CONTEXT

Variables in a template are evaluated in the current *context*. The
context at the top level of the template is called the *global
context*. This is the context provided to *tl_eval*(). Certain
directives modify the current context, i.e. by pushing a new context
and popping back the previous one later.

# SYNTAX

tl_html directives all begin with the character *@*. Any text that is
not part of a directive is output verbatim. To output an *@* character
verbatim, write *@@*.

## VARIABLES

A value from the current context (called a "variable") may be output
using the *@{variable}* directive. For example:

```
<h1>Name is @{user.name}</h1>
<b>The user's 3rd favorite color is @{user.favorite_colors.2}</b>
```

As shown above, *variable* is a sequence of *.*-separated names. If
the name is a base-10 integer it is interpreted as a call to
*tl_geti*(), otherwise to *tl_gets*().

If *variable* is empty (e.g. *@{}*) it evaluates to the current context.

*@{variable}* HTML escapes the text it produces, so it is safe to use
with unsanitized input. To insert text without escaping it, use
*@:{variable}*.

## IF

Conditional evaluation is possible using the *@?{variable}*
directive. *variable* is evaluated the same as above, and if it is
truthy (i.e. not _NULL_ or the integer 0) the template is output until
the next *@/*.

```
@?{user.email}
	<b>Users email is @{user.email}
@:
	<b>User does not have an email</b>
@/
```

The *@!{variable}* directive is like a *@?* except *variable* is
negated. There is no equivalent to an "else" clause.

## LOOP

Looping over a _TL_LIST_ is possible with the *@\*{variable}*
directive. *variable* is evaluated according to the rules above, and
if it is a list, the template until the next *@/* is evaluated once
per list item in the context of that item.

```
<b>Users:</b>
<ul>
	@*{users}
		<li>Name = @{name}, email = @{email}</li>
	@/
</ul>
```

# SEE ALSO

*tl_eval*(3)