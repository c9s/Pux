<?php
use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Testing\MuxTestCase;


class MuxConditionTest extends MuxTestCase
{

    public function testRouteWithDomainCondition() 
    {
        $mux = new Mux;
        $mux->add('/foo', ['HelloController2', 'indexAction'], ['domain' => 'test.dev']);

        $_SERVER['HTTP_HOST'] = 'test.dev';
        $route = $mux->dispatch('/foo');
        $this->assertNotNull($mux);

        $_SERVER['HTTP_HOST'] = 'github.com';
        $route = $mux->dispatch('/foo');
        $this->assertNull($route);
    }

    public function testMuxGetMethod()
    {
        $mux = new Mux;
        $mux->get('/news', ['NewsController', 'listAction']);
        $mux->get('/news_item', ['NewsController', 'itemAction'], []);

        $routes = $mux->getRoutes();
        $this->assertCount(2, $routes);
        $this->assertEquals(2, $mux->length());

        $_SERVER['REQUEST_METHOD'] = "GET";
        $this->assertNotNull($mux->dispatch('/news_item') );
        $this->assertNotNull( $mux->dispatch('/news') );
    }


}
