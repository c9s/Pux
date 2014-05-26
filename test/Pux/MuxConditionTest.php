<?php
use Pux\Mux;
use Pux\Executor;

class MuxConditionTest extends MuxTestCase
{

    public function testRouteWithDomainCondition() {
        $mux = new \Pux\Mux;
        ok($mux, "got mux");
        $mux->add('/foo', [ 'HelloController2','indexAction' ], [ 'domain' => 'test.dev' ]);

        $_SERVER['HTTP_HOST'] = 'test.dev';
        $route = $mux->dispatch('/foo');
        ok($route);

        $_SERVER['HTTP_HOST'] = 'github.com';
        $route = $mux->dispatch('/foo');
        ok(! $route);
    }

    public function testMuxGetMethod() {
        $mux = new \Pux\Mux;
        ok($mux);
        $mux->get('/news', [ 'NewsController','listAction' ]);
        $mux->get('/news_item', [ 'NewsController','itemAction' ], []);

        $routes = $mux->getRoutes();
        ok($routes);
        count_ok(2, $routes);
        is( 2, $mux->length() );

        $_SERVER['REQUEST_METHOD'] = "GET";
        ok( $mux->dispatch('/news_item') );
        ok( $mux->dispatch('/news') );
    }


}
