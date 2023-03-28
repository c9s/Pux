<?php
use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Testing\MuxTestCase;

class MuxBasicTest extends \PHPUnit\Framework\TestCase
{

    public function testCallbackGeneralize() 
    {
        $mux = new Mux;
        $mux->add('/', 'PageController:page1');
        ok($mux);
        $routes = $mux->getRoutes();
        ok($routes);
        ok( is_array($routes) );
        [$pcre, $pattern, $callback, $options] = $routes[0];
        ok( ! $pcre);
        ok( is_array($callback) );
        same_ok( ['PageController', 'page1'] , $callback );
    }

    public function testSubMuxExpand() 
    {
        $mainMux = new Mux;
        $mainMux->expand = true;

        $pageMux = new Mux;
        $pageMux->add('/page1', ['PageController', 'page1']);
        $pageMux->add('/page2', ['PageController', 'page2']);
        // $pageMux->add('/post/:id', [ 'PageController', 'page2' ]);

        $mainMux->mount('/sub', $pageMux);

        foreach( ['/sub/page1', '/sub/page2'] as $p ) {
            $r = $mainMux->dispatch($p);
            ok($r, sprintf('Matched route for %s', $p));
        }
    }

    public function testMuxMounting() {
        $mainMux = new Mux;
        $mainMux->expand = false;

        $pageMux = new Mux;
        $pageMux->add('/page1', ['PageController', 'page1']);
        $pageMux->add('/page2', ['PageController', 'page2']);

        $mainMux->mount('/sub', $pageMux);

        $pageMux2 = new Mux;
        $pageMux2->add('/bar', ['PageController', 'page1']);
        $pageMux2->add('/zoo', ['PageController', 'page2']);
        is( 2, $pageMux2->length());

        $mainMux->mount('/foo', $pageMux2);
        is( 2, $mainMux->length());

        foreach( ['/sub/page1', '/sub/page2', '/foo/bar', '/foo/zoo'] as $p ) {
            $r = $mainMux->dispatch($p);
            ok($r, sprintf('Matched route for %s', $p));
        }

        return $mainMux;
    }

    public function testNormalPattern() 
    {
        $mux = new Mux;
        $mux->add('/hello/show', ['HelloController', 'show']);

        $route = $mux->dispatch('/hello/show');
        ok($route, 'Found route');
        $response = RouteExecutor::execute($route);
        is("response", $response[2]);
    }

    public function testDispatchToRoot() 
    {
        $mux = new Mux;
        ok($mux);
        $mux->add('/', ['HelloController', 'index']);
        $route = $mux->dispatch('/');
        ok($route);
    }

    public function testRequirementPattern() 
    {
        $mux = new Mux;
        ok($mux);
        $mux->add('/hello/:name', ['HelloController', 'index'], ['require' => ['name' => '\w+']]);
        $route = $mux->dispatch('/hello/john');
        ok($route);
    }


    public function testPCREPattern()
    {
        $mux = new Mux;
        $mux->add('/hello/:name', ['HelloController', 'index']);

        $mux->compile("_cache.php");

        $route = $mux->dispatch('/hello/John');
        ok($route);
        ok($route[3]['vars'], 'vars');
        ok($route[3]['vars']['name'], 'vars.name');

        $response = RouteExecutor::execute($route);
        is("Hello John", $response[2]);

        if ( file_exists("_cache.php") ) {
            unlink("_cache.php");
        }
    }
}

