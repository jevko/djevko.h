# Whitespace

The Djevko parser automatically trims whitespace from:

1. opening tags, leading
2. closing tags, trailing
3. length-prefixes, leading
4. after length-prefixed strings, trailing
5. unquoted prefixes and suffixes, both leading and trailing

All of this is for convenience.

The first 4 cases make it possible to format [heredocs and length-prefixed strings](HEREDOCS.md) to be more human-readable and writable, so you are not required to smoosh everything together:

```
[...]tag`heredoc`tag[...]
```

but can format it nicely instead:

```
[...]
tag`heredoc`tag
[...]
```

If the newlines before the opening tag and the after the closing tag were significant, the tags wouldn't match -- opening would be `"\ntag"`, closing would be `"tag\n"`.

Now case 5. makes Djevko actually diverge from Jevko -- again, for convenience [in our particular use case](RATIONALE.md).

In the general case preserving all whitespace is sometimes useful -- in particular when using Jevko as a markup language.

Here though we want more of a data-interchange format.

Because of that, 99 % of the time we would strip this whitespace anyway, so the parser might as well do this for us on-the-fly -- which is of course faster. The remaining 1 % of the time we can simply quote/heredoc our strings:

```
` padded prefix ` [` padded suffix `]
```
