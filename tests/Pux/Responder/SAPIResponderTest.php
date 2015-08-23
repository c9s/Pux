<?php
use Pux\Responder\SAPIResponder;

class SAPIResponderTest extends PHPUnit_Framework_TestCase
{

    public function testStringResponse()
    {
        $fd = fopen('php://memory', 'w');
        $responder = new SAPIResponder($fd);
        $responder->respond('Hello World');
        fclose($fd);
    }


    public function testArrayResponse()
    {
        $fd = fopen('php://memory', 'w');
        $responder = new SAPIResponder($fd);
        $responder->respond([ 200, [ 'Content-Type: text/plain' ], 'Hello World' ]);
        fclose($fd);
    }
}

