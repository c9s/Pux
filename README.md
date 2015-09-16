Pux
=============
Pux is a faster PHP router, it also includes out-of-box controller helpers.

[![Latest Stable Version](https://poser.pugx.org/corneltek/pux/v/stable)](https://packagist.org/packages/corneltek/pux) 
[![Total Downloads](https://poser.pugx.org/corneltek/pux/downloads)](https://packagist.org/packages/corneltek/pux) 
[![Latest Unstable Version](https://poser.pugx.org/corneltek/pux/v/unstable)](https://packagist.org/packages/corneltek/pux) 
[![License](https://poser.pugx.org/corneltek/pux/license)](https://packagist.org/packages/corneltek/pux)

**2.0 Branch Build Status** *(This branch is under development)*

[![Coverage Status](https://coveralls.io/repos/c9s/Pux/badge.svg?branch=2.0&service=github)](https://coveralls.io/github/c9s/Pux?branch=2.0) [![Build Status](https://travis-ci.org/c9s/Pux.svg?branch=master)](https://travis-ci.org/c9s/Pux)

Benchmark
--------------------
- See <https://github.com/tyler-sommer/php-router-benchmark> for details


FEATURES
--------------------

- Low memory footprint (only 6KB with simple routes and extension installed) .
- Low overhead.
- PCRE pattern path support. (Sinatra-style syntax)
- Controller auto-mounting - you mount a controller automatically without specifying paths for each action.
- Controller annotation support - you may override the default path from controller through the annotations.
- Route with optional pattern.
- Request constraints
  - Request method condition support.
  - Domain condition support.
  - HTTPS condition support.

REQUIREMENT
--------------

- PHP 5.4+


INSTALLATION
--------------------

```sh
composer require corneltek/pux "2.0.x-dev"
```

SYNOPSIS
--------

The routing usage is dead simple:

```php
require 'vendor/autoload.php'; // use PCRE patterns you need Pux\PatternCompiler class.
use Pux\RouteExecutor;

class ProductController {
    public function listAction() {
        return 'product list';
    }
    public function itemAction($id) { 
        return "product $id";
    }
}
$mux = new Pux\Mux;
$mux->any('/product', ['ProductController','listAction']);
$mux->get('/product/:id', ['ProductController','itemAction'] , [
    'require' => [ 'id' => '\d+', ],
    'default' => [ 'id' => '1', ]
]);
$mux->post('/product/:id', ['ProductController','updateAction'] , [
    'require' => [ 'id' => '\d+', ],
    'default' => [ 'id' => '1', ]
]);
$mux->delete('/product/:id', ['ProductController','deleteAction'] , [
    'require' => [ 'id' => '\d+', ],
    'default' => [ 'id' => '1', ]
]);

// If you use ExpandableController, it will automatically expands your controller actions into a sub-mux
$mux->mount('/page', new PageController);

$submux = new Pux\Mux;
$submux->any('/bar');
$mux->mount('/foo',$submux); // mount as /foo/bar

// RESTful Mux Builder
$builder = new RESTfulMuxBuilder($mux, [ 'prefix' => '/=' ]);
$builder->addResource('product', new ProductResourceController); // expand RESTful resource point at /=/product
$mux = $builder->build();


if ($route = $mux->dispatch('/product/1')) {
    $response = RouteExecutor::execute($route);

    $responder = new Pux\Responder\SAPIResponder();
    // $responder->respond([ 200, [ 'Content-Type: text/plain' ], 'Hello World' ]);
    $responder->respond($response);
}
```


Mux
---
Mux is where you define your routes, and you can mount multiple mux to a parent one.

```php
$mainMux = new Mux;

$pageMux = new Mux;
$pageMux->any('/page1', [ 'PageController', 'page1' ]);
$pageMux->any('/page2', [ 'PageController', 'page2' ]);

// short-hand syntax
$pageMux->any('/page2', 'PageController:page2'  );

$mainMux->mount('/sub', $pageMux);

foreach( ['/sub/page1', '/sub/page2'] as $p ) {
    $route = $mainMux->dispatch($p);

    // The $route contains [ pcre (boolean), path (string), callback (callable), options (array) ]
    list($pcre, $path, $callback, $options) = $route;
}
```

### Methods

- `Mux->add( {path}, {callback array or callable object}, { route options })`
- `Mux->post( {path}, {callback array or callable object}, { route options })`
- `Mux->get( {path}, {callback array or callable object}, { route options })`
- `Mux->put( {path}, {callback array or callable object}, { route options })`
- `Mux->any( {path}, {callback array or callable object}, { route options })`
- `Mux->delete( {path}, {callback array or callable object}, { route options })`
- `Mux->mount( {path}, {mux object}, { route options })`
- `Mux->length()` returns length of routes.
- `Mux->export()` returns Mux constructor via __set_state static method in php code.
- `Mux->dispatch({path})` dispatch path and return matched route.
- `Mux->getRoutes()` returns routes array.
- `Mux::__set_state({object member array})` constructs and returns a Mux object.

### Sorting routes

You need to sort routes when not using compiled routes, it's because pux sorts
longer path to front:

```php
$pageMux = new Mux;
$pageMux->add('/', [ 'PageController', 'page1' ]);
$pageMux->add('/pa', [ 'PageController', 'page1' ]);
$pageMux->add('/page', [ 'PageController', 'page1' ]);
$pageMux->sort();
```

This sorts routes to:

```
/page
/pa
/
```

So pux first compares `/page`, `/pa`, than `/`.

### Different String Comparison Strategies

When expand is enabled, the pattern comparison strategy for 
strings will match the full string.

When expand is disabled, the pattern comparison strategy for 
strings will match the prefix.


RouteRequest
-------------------------

RouteRequest maintains the information of the current request environment, it
also provides some constraint checking methods that helps you to identify a
request, e.g.:

```php
if ($request->queryStringMatch(...)) {

}
if ($request->hostEqual('some.dev')) {

}
if ($request->pathEqual('/foo/bar')) {

}
```


```php
use Pux\Environment;
$env = Environment::createFromGlobals();
$request = RouteRequest::createFromEnv($env);

if ($route = $mux->dispatchRequest($request)) {

}
```


APCDispatcher
----------------------
Although Pux\\Mux is already fast, you can still add APCDispatcher to boost the
performance, which is to avoid re-lookup route.

This is pretty useful when you have a lot of PCRE routes.

```
use Pux\Dispatcher\APCDispatcher;
$dispatcher = new APCDispatcher($mux, array(
    'namespace' => 'app_',
    'expiry' => ...,
));
$route = $dispatcher->dispatch('/request/uri');
var_dump($route);
```

Controller
--------------------

Pux provides the ability to map your controller methods to paths automatically, done either through a simple, fast controller in the C extension or its pure PHP counterpart:

```php
class ProductController extends \Pux\Controller
{
    // translate to path ""
    public function indexAction() { }

    // translate to path "/add"
    public function addAction() { }

    // translate to path "/del"
    public function delAction() { }
}

$mux = new Pux\Mux;
$submux = $controller->expand();
$mux->mount( '/product' , $submux );

// or even simpler
$mux->mount( '/product' , $controller);

$mux->dispatch('/product');       // ProductController->indexAction
$mux->dispatch('/product/add');   // ProductController->addAction
$mux->dispatch('/product/del');   // ProductController->delAction
```

You can also use `@Route` and `@Method` annotations to override the default `\Pux\Controller::expand()` functionality:

```php
class ProductController extends \Pux\Controller
{
    /**
     * @Route("/all")
     * @Method("GET")
     */
    public function indexAction() {
        // now available via GET /all only
    }
    
    /**
     * @Route("/create")
     * @Method("POST")
     */
    public function addAction() {
        // now available via POST /create only
    }
    
    /**
     * @Route("/destroy")
     * @Method("DELETE")
     */
    public function delAction() {
        // now available via DELETE /destroy only
    }
}
```

This is especially helpful when you want to provide more specific or semantic
(e.g., HTTP method-specific) actions.  Note that by default, expanded
controller routes will be available via any HTTP method - specifying `@Method`
will restrict it to the provided method.

- `Pux\Controller::expand()` returns an instance of `\Pux\Mux` that contains
  the controller's methods mapped to URIs, intended to be mounted as a sub mux
  in another instance of `\Pux\Mux`.


Route RouteExecutor
--------------------
`Pux\RouteExecutor` executes your route by creating the controller object, and
calling the controller action method.

Route executor take the returned route as its parameter, you simply pass the
route to executor the controller and get the execution result.

Here the simplest example of the usage:

```php
use Pux\RouteExecutor;
$mux = new Pux\Mux;
$mux->any('/product/:id', ['ProductController','itemAction']);
$route = $mux->dispatch('/product/1');
$result = RouteExecutor::execute($route);
```

You can also define the arguments to the controller's constructor method:

```php

class ProductController extends Pux\Controller {
    public function __construct($param1, $param2) {
        // do something you want
    }
    public function itemAction($id) {
        return "Product $id";
    }
}

use Pux\RouteExecutor;
$mux = new Pux\Mux;
$mux->any('/product/:id', ['ProductController','itemAction'], [ 
    'constructor_args' => [ 'param1', 'param2' ],
]);
$route = $mux->dispatch('/product/1');
$result = RouteExecutor::execute($route); // returns "Product 1"
```

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


Routing Path Format
---------------------

Static route:

    /post

PCRE route:

    /post/:id                  => matches /post/33

PCRE route with optional pattern:

    /post/:id(/:title)         => matches /post/33, /post/33/post%20title
    /post/:id(\.:format)       => matches /post/33, /post/33.json .. /post/33.xml




## Q & A

### Why It's Faster

- Pux uses simpler data structure (indexed array) to store the patterns and flags.
    (In PHP internals, `zend_hash_index_find` is faster than `zend_hash_find`).

- When matching routes, symfony uses a lot of function calls for each route:

    https://github.com/symfony/Routing/blob/master/Matcher/UrlMatcher.php#L124

    Pux fetches the pattern from an indexed-array:

    https://github.com/c9s/Pux/blob/master/src/Pux/Mux.php#L189

- Even you enabled APC or other bytecode cache extension, you are still calling
  methods and functions in the runtime. Pux reduces the route building to one
  static method call. `__set_state`.

- Pux separates static routes and dynamic routes automatically, Pux uses hash
  table to look up static routes without looping the whole route array.

- Pux\\Mux is written in C extension, method calls are faster!

- With C extension, there is no class loading overhead.

- Pux compiles your routes to plain PHP array, the compiled routes can be
  loaded very fast. you don't need to call functions to register your routes before using it.


### Why It's Here

Most of us use a lot of machines to run our applications, however, it uses too
much energy and too many resources.

Some people thinks routing is not the bottleneck, the truth is this project
does not claim routing is the bottleneck.

Actually the "bottleneck" is always different in different applications, if you
have a lot of heavy db requests, then your bottleneck is your db; if you have a
lot of complex computation, then the bottleneck should be your algorithm.

You might start wondering since the bottleneck is not routing, why do we
implement route dispatcher in C extension? The answer is simple, if you put a
pure PHP routing component with some empty callbacks and use apache benchmark
tool to see how many requests you can handle per second, you will find out the
routing component consumes a lot of computation time and the request number
will decrease quite a few. (and it does nothing, all it does is ... just
routing)

Pux tries to reduce the overheads of loading PHP classes and the runtime
method/function calls, and you can run your application faster without the
overheads.

### Pros & Cons of Grouped Pattern Matching Strategy

An idea of matching routes is to combine all patterns into one pattern and
compare the given path with `pcre_match` in one time.

However this approach does not work if you have optional group or named
capturing group, the `pcre_match` can not return detailed information about
what pattern is matched if you use one of them.

And since you compile all patterns into one, you can't compare with other same
patterns with different conditions, for example:

    /users  # GET
    /users  # POST
    /users  # with HTTP_HOST=somedomain

The trade off in Pux is to compare routes in sequence because the same pattern
might be in different HTTP method or different host name.

The best approach is to merge & compile the regexp patterns into a FSM (Finite
state machine), complex conditions can also be merged into this FSM, and let
this FSM to dispatch routes. And this is the long-term target of Pux.






## Contributing

### Testing XHProf Middleware


Define your XHPROF_ROOT in your `phpunit.xml`, you can copy `phpunit.xml.dist` to `phpunit.xml`,
for example:

```xml
  <php>
    <env name="XHPROF_ROOT" value="/Users/c9s/src/php/xhprof"/>
  </php>
```




### Hacking Pux C extension

1. Discuss your main idea on GitHub issue page.

2. Fork this project and open a branch for your hack.

3. Development Cycle:

        cd ext
        ./compile
        ... hack hack hack ...

        # compile and run phpunit test
        ./compile && ./test -- --debug tests/Pux/MuxTest.php

        # use lldb to debug extension code
        ./compile && ./test -l -- tests/Pux/MuxTest.php

        # use gdb to debug extension code
        ./compile && ./test -g -- tests/Pux/MuxTest.php

4. Commit!

5. Send pull request and describe what you've done and what is changed.

