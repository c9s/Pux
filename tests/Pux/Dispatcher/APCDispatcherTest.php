<?php
use Pux\Dispatcher\APCDispatcher;
use Pux\Mux;

class APCDispatcherTest extends PHPUnit_Framework_TestCase
{
    public function test()
    {
        $mux = new Mux;
        $mux->add('/product/add', ['ProductController', 'addAction']);
        $dispatcher = new APCDispatcher($mux, array(
            'namespace' => 'tests',
            'expiry' => 10,
        ));
        ok($dispatcher);
        $route = $dispatcher->dispatch('/product/add');
        ok($route);
        $route = $dispatcher->dispatch('/product/add');
        ok($route);

        $route = $dispatcher->dispatch('/product/del');
        ok($route == false);
    }
}

