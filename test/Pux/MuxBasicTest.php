<?php
use Pux\Mux;
use Pux\Executor;

class MuxBasicTest extends PHPUnit_Framework_TestCase
{

    public function testCallbackGeneralize() 
    {
        $m = new Mux;
        $m->add('/', 'PageController:page1');
        ok($m);
        $routes = $m->getRoutes();
        ok($routes);
        ok( is_array($routes) );
        $route = $routes[0];
        list($pcre, $pattern, $callback, $options) = $route;
        ok( ! $pcre);
        ok( is_array($callback) );
        same_ok( array('PageController','page1') , $callback );
    }

    public function testSubMuxExpand() 
    {
        $mainMux = new Mux;
        $mainMux->expand = true;

        $pageMux = new Mux;
        $pageMux->add('/page1', array( 'PageController', 'page1' ));
        $pageMux->add('/page2', array( 'PageController', 'page2' ));
        // $pageMux->add('/post/:id', [ 'PageController', 'page2' ]);

        $mainMux->mount('/sub', $pageMux);

        foreach( array('/sub/page1', '/sub/page2') as $p ) {
            $r = $mainMux->dispatch($p);
            ok($r, "Matched route for $p");
        }
    }

    public function testMuxMounting() {
        $mainMux = new Mux;
        $mainMux->expand = false;

        $pageMux = new Mux;
        $pageMux->add('/page1', array( 'PageController', 'page1' ));
        $pageMux->add('/page2', array( 'PageController', 'page2' ));
        $mainMux->mount('/sub', $pageMux);

        $pageMux2 = new Mux;
        $pageMux2->add('/bar', array( 'PageController', 'page1' ));
        $pageMux2->add('/zoo', array( 'PageController', 'page2' ));
        is( 2, $pageMux2->length());

        $mainMux->mount('/foo', $pageMux2);
        is( 2, $mainMux->length());

        foreach( array('/sub/page1', '/sub/page2', '/foo/bar', '/foo/zoo') as $p ) {
            $r = $mainMux->dispatch($p);
            ok($r, "Matched route for $p");
        }
        return $mainMux;
    }


    /**
     * @depends testMuxMounting
     */
    public function testSubMuxExport($mainMux)
    {
        $code = '$newMux = ' . $mainMux->export() . ';';
        ok($code, 'code');
        eval($code);
        ok($newMux);
        foreach( ['/sub/page1', '/sub/page2', '/foo/bar', '/foo/zoo'] as $p ) {
            $r = $newMux->dispatch($p);
            ok($r, "Matched route for $p");
        }
    }


    public function testNormalPattern() 
    {
        $mux = new Mux;
        ok($mux);

        $mux->add('/hello/show', [ 'HelloController', 'show' ]);

        $route = $mux->dispatch('/hello/show');
        ok($route, 'Found route');
        $response = Executor::execute($route);
        is("response", $response);
    }

    public function testDispatchToRoot() 
    {
        $mux = new Mux;
        ok($mux);
        $mux->add('/', [ 'HelloController', 'index' ]);
        $route = $mux->dispatch('/');
        ok($route);
    }

    public function testRequirementPattern() 
    {
        $mux = new Mux;
        ok($mux);
        $mux->add('/hello/:name', [ 'HelloController', 'index' ], [
            'require' => [ 'name' => '\w+' ],
        ]);
        $route = $mux->dispatch('/hello/john');
        ok($route);
    }


    public function testPCREPattern()
    {
        $mux = new Mux;
        ok($mux);

        $mux->add('/hello/:name', [ 'HelloController', 'index' ]);

        $mux->compile("_cache.php");

        $route = $mux->dispatch('/hello/John');
        ok($route);
        ok($route[3]['vars'], 'vars');
        ok($route[3]['vars']['name'], 'vars.name');

        $response = Executor::execute($route);
        is("Hello John", $response);

        if ( file_exists("_cache.php") ) {
            unlink("_cache.php");
        }
    }
}

