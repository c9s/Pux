Pux
=============
Pux is a high performance PHP router.

Pux is 48.5x faster than symfony router in static route dispatching, 31x faster in regular expression dispatching. (with pux extension installed)

(Benchmark details here https://github.com/c9s/router-benchmark/blob/master/code)

Pux tries not to consume computation time to build all routes dynamically (like
Symfony/Routing, although the RouteCompiler of Symfony/Routing caches the
compiled patterns, but there are still a lot of function call and class
loading from your application code. however, function calls are pretty slow in PHP). 

Why It's Faster
---------------

- Pux uses simpler data structure (indexed array) to store the patterns and flags.
    (In PHP internals, `zend_hash_index_find` is faster than `zend_hash_find`).
    when matching routes, symfony uses a lot of function calls for each routes:

    https://github.com/symfony/Routing/blob/master/Matcher/UrlMatcher.php#L124

    Pux fetches the pattern from an indexed-array:

    https://github.com/c9s/Pux/blob/master/src/Pux/Mux.php#L189

- Pux separates static routes and dynamic routes automatically, Pux uses hash
  table to look up static routes without looping the whole routes.

- Pux\\Mux is written in C extension.

- Pux compiles your routes to plain PHP array, the compiled routes can be
  loaded very fast. you don't need to call functions to register your routes before using it.

Features
--------------------

- Zero dependency.
- Low memory footprint (only 6KB with simple routes and extension installed) .
- High performance of dispatching routes.
- PCRE pattern path support. (Sinatra-style)
- Request method condition support.
- Domain condition support.
- https condition support.

Routing Path Format
---------------------

    /post
    /post/:id                  => matches /post/33
    /post/:id(/:title)         => matches /post/33, /post/33/post%20title
    /post/:id(\.:format)       => matches /post/33, /post/33.json .. /post/33.xml

Installation
--------------------
You can install Pux with composer by defining the following requirement in your composer.json:

    {
        "require": {
            "corneltek/pux": "~1.2"
        }
    }

To install pux extension to boost the performance:

    git clone https://github.com/c9s/Pux.git
    cd Pux/ext
    phpize
    ./configure
    make && make install

Then setup your php.ini config to load pux extension:

    extension=pux.so


Synopsis
------------

The routing usage is dead simple:

```php
require 'vendor/autoload.php'; // use PCRE patterns you need Pux\PatternCompiler class.
use Pux\Executor;

class ProductController {
    public function listAction() {
        return 'product list';
    }
    public function itemAction($id) { 
        return "product $id";
    }
}
$mux = new Pux\Mux;
$mux->add('/product', ['ProductController','listAction']);
$mux->add('/product/:id', ['ProductController','itemAction'] , [
    'require' => [ 'id' => '\d+', ],
    'default' => [ 'id' => '1', ]
]);
$route = $mux->dispatch('/product/1');
Executor::execute($route);
```


Examples
--------------------

### Basic Example

```php
require 'vendor/autoload.php';
use Pux\Mux;
use Pux\Executor;
$mux = new Mux;
$mux->get('/get', ['HelloController','helloAction']);
$mux->post('/post', ['HelloController','helloAction']);
$mux->put('/put', ['HelloController','helloAction']);
$route = $mux->dispatch( $_SERVER['PATH_INFO'] );
echo Executor::execute($route);
```

### Through Compiled Mux

Define your routing definition in `routes.php`:

```php
require 'vendor/autoload.php';
use Pux\Mux;
$mux = new Mux;
$mux->get('/get', ['HelloController','helloAction']);
return $mux;
```

Run pux command to compile your routing definition:

```sh
pux compile -o mux.php routes.php
```

Load the mux object from your application code:

```php
require 'vendor/autoload.php';
$mux = require 'mux.php';
$route = $mux->dispatch( $_SERVER['PATH_INFO'] );
echo Executor::execute($route);
```

Mux
-----
Mux is where you define your routes, and you can mount multiple mux to a parent one.

```php
$mainMux = new Mux;
$mainMux->expand = true;

$pageMux = new Mux;
$pageMux->add('/page1', [ 'PageController', 'page1' ]);
$pageMux->add('/page2', [ 'PageController', 'page2' ]);

$mainMux->mount('/sub', $pageMux);

foreach( ['/sub/page1', '/sub/page2'] as $p ) {
    $r = $mainMux->dispatch($p);
    ok($r, "Matched route for $p");
}
```

The `expand` option means whether to expand/merge submux routes to the parent mux.

When expand is enabled, it improves dispatch performance when you
have a lot of sub mux to dispatch.

### Different String Comparison Strategies

When expand is enabled, the pattern comparison strategy for 
strings will match the full string.

When expand is disabled, the pattern comparison strategy for 
strings will match the prefix.



MuxCompiler
--------------------

In your route definition file `hello_routes.php`, you simply return the Mux object at the end of file:

```php
<?php
// load your composer autoload if it's needed
// require '../vendor/autoload.php';
use Pux\Mux;
use Pux\Executor;
$mux = new Mux;
$mux->get('/hello', ['HelloController','helloAction']);
return $mux;
```

Pux provides a command-line tool for you to compile your route definitions.

    pux compile -o hello_mux.php hello_routes.php

In your application, you may load the compiled mux (router) through only one line:

```php
<?php
$mux = require "hello_mux.php";
$route = $mux->dispatch('/hello');
```

This can be very very fast if you have pux extension installed.

Dispatching Strategy
--------------------

There are two route dispatching strategies in Pux while Symfony/Routing only
provides PCRE pattern matching:

1. Plain string comparison.
2. PCRE pattern comparison.

You've already knew that PCRE pattern matching is slower than plain string comparison, although PHP PCRE caches the compiled patterns.

The plain string comparison is designed for static routing paths, it
improves the performance while you have a lot of simple routes.

The PCRE pattern comparison is used when you have some dynamic routing paths,
for example, you can put some place holders in your routing path, and pass
these path arguments to your controller later.

Pux sorts and compiles your routes to single cache file, it also uses longest
matching so it sorts patterns by pattern length in descending order before compiling the
routes to cache.

Pux uses indexed array as the data structure for storing route information so it's faster.



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


### Dispatch Speed

With one static route:

    n=10000
    Runing pux extension (dispatch) - . 97487.768426386/s
    Runing symfony/routing (dispatch) - . 2456.3512428418/s
    
                                    Rate   Mem pux extension (dispatch) symfony/routing (dispatch)
      pux extension (dispatch)  97.49K/s    0B                       --                        -2%
    symfony/routing (dispatch)   2.46K/s  524K                    3968%                         --
    
    
    ================================== Bar Chart ==================================
    
        pux extension (dispatch)  97.49K/s | ████████████████████████████████████████████████████████████  |
      symfony/routing (dispatch)   2.46K/s | █                                                             |
    
    
    ============================== System Information ==============================
    
    PHP Version: 5.5.6
    CPU Brand String: Intel(R) Core(TM) i5-3427U CPU @ 1.80GHz
    
    With XDebug Extension.

With one pcre route:

    n=5000
    Runing pux extension (dispatch) - . 68264.888935184/s
    Runing symfony/routing (dispatch) - . 2245.5539220463/s
    
                                    Rate   Mem pux extension (dispatch) symfony/routing (dispatch)
      pux extension (dispatch)  68.26K/s    3M                       --                        -3%
    symfony/routing (dispatch)   2.25K/s  786K                    3040%                         --
    
    
    ================================== Bar Chart ==================================
    
        pux extension (dispatch)  68.26K/s | ████████████████████████████████████████████████████████████  |
      symfony/routing (dispatch)   2.25K/s | █                                                             |
    
    
    ============================== System Information ==============================
    
    PHP Version: 5.5.6
    CPU Brand String: Intel(R) Core(TM) i5-3427U CPU @ 1.80GHz

### Through Apache

Please see benchmark details here: <https://github.com/c9s/router-benchmark>
