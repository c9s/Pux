<?php
use Pux\Mux;
use Pux\Testing\MuxTestCase;


class MuxDispatchTest extends PHPUnit_Framework_TestCase
{
    public function testMuxCustomDispatch()
    {
        $mux = new Mux;
        $mux->mount('/bs/product', function($x) { 
            $x = new Mux;
            $x->any('/create', ['Product', 'createAction']);
            $x->any('/edit/:id', ['Product', 'editAction']);
            return $x;
        });

        $route = $mux->dispatch('/bs/product/create');
        $this->assertNotEmpty($route);
        $this->assertEquals('/bs/product/create', $route[1]);

        $route = $mux->dispatch('/bs/product/edit/30');
        $this->assertNotEmpty($route);
        $this->assertEquals('/bs/product/edit/:id', $route[3]['pattern']);
    }
}
