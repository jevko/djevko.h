# Heredocs

Say we have a djevko such as:

```
allowed characters [0123456789]
```

And we would want to add `[` and `]` to `allowed characters` without them being interpreted as special. We can do this simply by surrounding the characters with `` ` ``:

```
allowed characters [`0123456789[]`]
```

We could even include the `` ` `` character itself, provided that we don't put it immediately before the quoted `]`:

```
allowed characters [`0123456789[]``]
```

What if we had to put it before the `]` though? Simply precede the opening `` ` `` with a [tag of your choice](#rules-for-tags) and put a matching tag after the closing `` ` ``:

```
allowed characters [=`0123456789[`]`=]
```

Here we chose `=` as the tag. Now if we wanted to put a `=` before the `]` as well, we simply adjust the tag:

```
allowed characters [==`0123456789[`=]`==]
```

Here we chose `==` as the tag.

## Rules for tags

We can pick any tag we like, provided it doesn't contain the special characters `[`, `]`, or `` ` ``.

Tags which are integers (e.g. `10`) are exceptional -- they indicate [length-prefixed strings](#length-prefixed-strings) which work differently than heredocs described above.

## Notes about whitespace in tags

Note that whitespace that precedes the opening tag is not counted as part of it, so this:

```
allowed characters [  ==`0123456789[`=]`==]
```

is equivalent to the above. Similarly, whitespace that follows the closing tag does not count:

```
allowed characters [==`0123456789[`=]`==  ]
```

Also, whitespace inbetween the tag and the `` ` `` on either side is not allowed. 

❌ So the following won't work:

```
allowed characters [==  `0123456789[`=]`==]
```

❌ Neither will this:

```
allowed characters [==`0123456789[`=]`  ==]
```

Finally, whitespace inside the tag is allowed and considered part of the tag. So all of the below are correct and equivalent:

```
allowed characters [= =`0123456789[`=]`= =]

allowed characters [  = =`0123456789[`=]`= =]

allowed characters [= =`0123456789[`=]`= =  ]

allowed characters [  = =`0123456789[`=]`= =  ]
```

The tag here is `= =`.

## Characters that are considered whitespace

Whether a byte qualifies as whitespace is determined by [isspace](https://cplusplus.com/reference/cctype/isspace/). Therefore the following characters are considered whitespace:

```
' ' 	(0x20)	space (SPC)
'\t'	(0x09)	horizontal tab (TAB)
'\n'	(0x0a)	newline (LF)
'\v'	(0x0b)	vertical tab (VT)
'\f'	(0x0c)	feed (FF)
'\r'	(0x0d)	carriage return (CR)
```

# Length-prefixed strings

We have described how heredocs work.

Now, an alternative way to prevent the special characters `[`, `]`, and `` ` `` from being interpreted as such is length-prefixing.

It's similar to heredocs, except we only need a tag at the beginning and we don't need to worry at all if it occurs in the string that follows.

With this mechanism we can append `` [`] `` to `allowed characters` like so:

```
allowed characters [13`0123456789[`]]
```

The tag here is the number `13` which is also the length of the string `` 0123456789[`] `` -- this instructs the parser to interpret those 13 characters as a single string without looking at any special characters in it.

Whitespace before the tag and after the string is allowed, so this works too:

```
allowed characters [   13`0123456789[`]   ]
```

In contrast to heredocs, in length-prefixed strings we don't include the closing `` ` `` and we don't repeat the tag at the end.

❌ So this won't work:

```
allowed characters [13`0123456789[`]`13]
```

Like in heredocs, whitespace between the tag and the `` ` `` is not allowed.

❌ So this won't work either:

```
allowed characters [13 `0123456789[`]]
```

## Cons and pros of length-prefixed strings

Length-prefixed strings are short and simple, but they are cumbersome for manual editing: you are required to always know the exact length of your string. If you edit it, you must edit the length as well.

So what's the advantage? It's much faster for machine-generated data that doesn't need to be edited. When processing length-prefixed strings, the parser doesn't need to go through the source character-by-character -- it can simply skip over the specified number of characters all at once and then carry on as usual. 

So length-prefixed strings can work well for storing binary data, as a much faster alternative to base64 or the like.

Keep in mind however that most text editors won't be able to edit your binary files directly without corrupting them. Reading however will might still be possible.