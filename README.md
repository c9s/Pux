Phux
=============
The Fast PHP router.


Phux tries not to consume computation time to build all routes dynamically (like Symfony/Routing). Instead,
Phux compiles your routes to plain PHP array for caching, the compiled routes can be loaded from cache very fast.

There are two route dispatching strategy in Phux while Symfony/Routing only
provides PCRE pattern matching:

1. Plain string comparison.
2. PCRE pattern comparison.

You've already knew that PCRE pattern matching is slower than plain string comparison, although PHP PCRE caches the pattern.

The plain string comparison is designed for these static routing paths, it does
improve the performance while you have simple routes.

The PCRE pattern comparison is used when you have some dynamic routing paths,
for example, you can put the some place holder in your routing path, and pass
these path arguments to your controller later.

Phux sorts and compiles your route to single cache, we use longest matching so
we sort long patterns in descending order before compiling the routes to cache.

Phux uses indexed array as the data structure for storing route information so it's faster.


