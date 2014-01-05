Phux
=============
Phux is a High Performance PHP Router.

Phux tries not to consume computation time to build all routes dynamically (like Symfony/Routing). Instead,
Phux compiles your routes to plain PHP array for caching, the compiled routes can be loaded from cache very fast.

With Phux PHP Extension support, you may load and dispatch the routes 2x faster than pure PHP Phux.

MuxCompiler
--------------------

Phux provides a command-line tool for you to compile your route definitions.

    phux compile -o hello_mux.php hello_routes.php

In your route definition file, you simply return the Mux object at the end of file:

```php
<?php
// load your composer autoload if it's needed
// require '../vendor/autoload.php';
use Phux\Mux;
$mux = new Mux;
$mux->get('/hello', ['HelloController','helloAction']);
return $mux;
```

In your application, you may load the compiled mux (router) through only one line:

```php
<?php
$mux = require "hello_mux.php";
$route = $mux->dispatch('/hello');
```

This can be very very fast if you have phux extension installed.

Dispatching Strategy
--------------------

There are two route dispatching strategies in Phux while Symfony/Routing only
provides PCRE pattern matching:

1. Plain string comparison.
2. PCRE pattern comparison.

You've already knew that PCRE pattern matching is slower than plain string comparison, although PHP PCRE caches the compiled patterns.

The plain string comparison is designed for static routing paths, it
improves the performance while you have a lot of simple routes.

The PCRE pattern comparison is used when you have some dynamic routing paths,
for example, you can put some place holders in your routing path, and pass
these path arguments to your controller later.

Phux sorts and compiles your routes to single cache file, it also uses longest
matching so it sorts patterns by pattern length in descending order before compiling the
routes to cache.

Phux uses indexed array as the data structure for storing route information so it's faster.


Synopsis
------------

The routing usage is dead simple:

```php
class ProductController {
    public function listAction() {
        return 'product list';
    }
    public function itemAction($id) { 
        return "product $id";
    }
}
$mux = new Phux\Mux;
$mux->add('/product', ['ProductController','listAction']);
$mux->add('/product/:id', ['ProductController','itemAction'] , [
    'require' => [ ':id' => '\d+', ],
    'default' => [ ':id' => '1', ]
]);
$route = $mux->dispatch('/product/1');
```

Mux
-----
Mux is where you define your routes, and you can mount multiple mux to a parent one.

```php
$mainMux = new Mux;
$mainMux->expandSubMux = true;

$pageMux = new Mux;
$pageMux->add('/page1', [ 'PageController', 'page1' ]);
$pageMux->add('/page2', [ 'PageController', 'page2' ]);

$mainMux->mount('/sub', $pageMux);

foreach( ['/sub/page1', '/sub/page2'] as $p ) {
    $r = $mainMux->dispatch($p);
    ok($r, "Matched route for $p");
}
```

The `expandSubMux` option means whether to expand/merge submux routes to the parent mux.

When expandSubMux is enabled, it improves dispatch performance when you
have a lot of sub mux to dispatch.

### Different String Comparison Strategies

When expandSubMux is enabled, the pattern comparison strategy for 
strings will match the full string.

When expandSubMux is disabled, the pattern comparison strategy for 
strings will match the prefix.


## Benchmarks

Testing with route dispatch only. (no controller)

Hardware:

- iMac Mid 2011
- Processor  2.5 GHz Intel Core i5
- Memory  12 GB 1333 MHz DDR3
- Software  OS X 10.9.1 (13B42)

Environment:

- Apache 2.2 + prefork worker
- PHP 5.5.6

Prefork configuration:

    StartServers          2
    MinSpareServers       3
    MaxSpareServers       3
    MaxClients           30
    MaxRequestsPerChild  1000


### Requests per seconds

<img src="https://raw.github.com/c9s/Phux/master/benchmarks/reqs.png"/>

### Response Time

Phux - PURE PHP (around 3ms~38ms)

<img src="https://raw.github.com/c9s/Phux/master/benchmarks/phux.png"/>

Phux - with extension 

<https://gist.github.com/c9s/8273098>

Symfony/Routing (around 9ms~146ms)

<img src="https://raw.github.com/c9s/Phux/master/benchmarks/symfony-routing.png"/>


