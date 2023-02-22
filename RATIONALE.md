# Rationale

I started sketching out Djevko because of a gamedev project in C for which Jevko seemed like a good use-case.

The requirements were:

* a single-header parser library
* nice if it could work with bytes 
* nice if it could represent prefixes and suffixes as simple (index + length) slices.

So I started thinking how that would look like, with this particular use case in mind.

Backward-compatibility with Jevko was not important at this point, especially that it became apparent that digraph-based escapes complicated the slicing.

So I came up with a way to achieve that in a minimal way with a variant of heredocs and length-prefixing.

And so here we are.