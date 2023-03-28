<?php
use Pux\Mux;
use Pux\RouteExecutor;
use Pux\Testing\MuxTestCase;

class MuxSetStateTest extends MuxTestCase
{

    public function testNullStaticRoutes() {
        $mux = Mux::__set_state(['id' => NULL, 'routes' =>
        [0 =>
            [0 => false, 1 => '/hello', 2 =>
            [0 => 'HelloController2', 1 => 'helloAction'], 3 => []]], 'submux' => [], 'staticRoutes' => [], 'routesById' => [], 'expand' => true]);
        ok($mux);
    }

}
