<?php
use Phux\Mux;
use Phux\Executor;

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

    public function testSubMux()
    {
        $mainMux = new Mux;

        $pageMux = new Mux;
        $pageMux->add('/page1', [ 'PageController', 'page1' ]);
        $pageMux->add('/page2', [ 'PageController', 'page2' ]);
        $mainMux->mount('/sub', $pageMux);

        $mainMux->dispatch('/sub/page1');
    }


    public function testNormalPattern() 
    {
        $mux = new Mux;
        ok($mux);

        $result = $mux->add('/hello/show', [ 'HelloController', 'show' ]);
        ok($result, 'Route added');

        $route = $mux->dispatch('/hello/show');
        ok($route, 'Found route');
        $response = Executor::execute($route);
        is("response", $response);
    }


    public function testPCREPattern()
    {
        $mux = new Mux;
        ok($mux);

        $result = $mux->add('/hello/:name', [ 'HelloController', 'index' ]);
        ok($result);

        $route = $mux->dispatch('/hello/John');
        ok($route);
        $response = Executor::execute($route);
        is("Hello John", $response);
    }
}

