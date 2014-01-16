<?php
use Pux\Mux;
use Pux\Executor;

class HelloController
{
    public function index($name) {
        return "Hello $name";
    }

    public function show() {
        return "response";
    }
}

class PageController {

    public function page1() { return 'page1'; }
    public function page2() { return 'page2'; }

}

class MuxTest extends PHPUnit_Framework_TestCase
{

    public function testSubMuxExpand() 
    {
        $mainMux = new Mux;
        $mainMux->expand = true;

        $pageMux = new Mux;
        $pageMux->add('/page1', [ 'PageController', 'page1' ]);
        $pageMux->add('/page2', [ 'PageController', 'page2' ]);
        // $pageMux->add('/post/:id', [ 'PageController', 'page2' ]);

        $mainMux->mount('/sub', $pageMux);

        foreach( ['/sub/page1', '/sub/page2'] as $p ) {
            $r = $mainMux->dispatch($p);
            ok($r, "Matched route for $p");
        }
    }

    public function testSubMuxExport()
    {
        $mainMux = new Mux;
        $mainMux->expand = false;

        $pageMux = new Mux;
        $pageMux->add('/page1', [ 'PageController', 'page1' ]);
        $pageMux->add('/page2', [ 'PageController', 'page2' ]);
        $mainMux->mount('/sub', $pageMux);

        $pageMux2 = new Mux;
        $pageMux2->add('/bar', [ 'PageController', 'page1' ]);
        $pageMux2->add('/zoo', [ 'PageController', 'page2' ]);
        is( 2, $pageMux2->length());

        $mainMux->mount('/foo', $pageMux2);
        is( 2, $mainMux->length());

        foreach( ['/sub/page1', '/sub/page2', '/foo/bar', '/foo/zoo'] as $p ) {
            $r = $mainMux->dispatch($p);
            ok($r, "Matched route for $p");
        }

        $code = '$newMux = ' . $mainMux->export() . ';';
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

