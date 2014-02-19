<?php
use Pux\Mux;
use Pux\Executor;

class MuxSetStateTest extends MuxTestCase
{

    public function testNullStaticRoutes() {
        $mux = Mux::__set_state(array(
            'id' => NULL,
            'routes' =>
            array (
                0 =>
                    array (
                        0 => false,
                        1 => '/hello',
                        2 =>
                        array (
                            0 => 'HelloController2',
                            1 => 'helloAction',
                        ),
                        3 => array (),
                ),
            ),
            'submux' => array(),
            'staticRoutes' => array(),
            'routesById' => array(),
            'expand' => true,
        ));
        ok($mux);
    }

}
