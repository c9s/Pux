<?php
use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Testing\MuxTestCase;


class MuxRouteExecutorTest extends MuxTestCase
{

    public function testRouteExecutor() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->add('/hello/:name', ['HelloController2', 'helloAction'], ['require' => ['name' => '\w+']]);
        $mux->add('/product/:id', ['ProductController', 'itemAction']);
        $mux->add('/product', ['ProductController', 'listAction']);
        $mux->add('/foo', ['ProductController', 'fooAction']);
        $mux->add('/bar', ['ProductController', 'barAction']);
        $mux->add('/', ['ProductController', 'indexAction']);

        ok( $r = $mux->dispatch('/') );

        $response = RouteExecutor::execute($r);
        $this->assertEquals('index', $response[2]);

        ok( $r = $mux->dispatch('/foo') );

        $response = RouteExecutor::execute($r);
        $this->assertEquals('foo', $response[2]);

        ok( $r = $mux->dispatch('/bar') );

        $response = RouteExecutor::execute($r);
        $this->assertEquals('bar', $response[2]);
    }
}
