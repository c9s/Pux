<?php
use Pux\Mux;
use Pux\Executor;

abstract class MuxTestCase extends PHPUnit_Framework_TestCase
{

    public function assertNonPcreRoute($route, $path = null) {
        $this->assertFalse($route[0], "Should be Non PCRE Route, Got: {$route[1]}");
        if ( $path ) {
            $this->assertSame($path, $route[1], "Should be $path, Got {$route[1]}");
        }
    }

    public function assertPcreRoute($route, $path = null) {
        $this->assertTrue($route[0], "Should be PCRE Route, Got: {$route[1]}" );
        if ( $path ) {
            $this->assertSame($path, $route[3]['pattern'], "Should be $path, Got {$route[1]}");
        }
    }
}
