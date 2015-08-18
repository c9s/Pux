<?php
use Pux\RouteRequest;

class RouteRequestTest extends PHPUnit_Framework_TestCase
{
    public function testCreateFromGlobals()
    {
        $request = RouteRequest::createFromGlobals();
        $this->assertNotNull($request);
    }
}

