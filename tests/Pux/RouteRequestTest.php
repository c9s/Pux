<?php
use Pux\RouteRequest;

class RouteRequestTest extends PHPUnit_Framework_TestCase
{
    public function testCreateFromGlobals()
    {
        $request = RouteRequest::createFromGlobals('GET', '/foo/bar');
        $this->assertNotNull($request);
    }

    public function createRequestMethodConstraint()
    {
        $request = RouteRequest::createFromGlobals('GET', '/foo/bar');
        $this->assertNotNull($request);

        $this->assertTrue($this->useRequestMethod('GET'));

    }
}

