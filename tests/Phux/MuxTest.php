<?php
use Phux\Mux;
use Phux\Executor;

class HelloController
{
    public function index($name) {
        return "Hello $name";
    }
}

class MuxTest extends PHPUnit_Framework_TestCase
{
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

