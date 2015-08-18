<?php
use Pux\RouteRequest;

class RouteRequestTest extends PHPUnit_Framework_TestCase
{
    public function testCreateFromGlobals()
    {
        $request = RouteRequest::createFromGlobals('GET', '/foo/bar');
        $this->assertNotNull($request);
    }

    public function testRequestUrl()
    {
        $request = RouteRequest::createFromGlobals('GET', '/foo/bar');
        $this->assertNotNull($request);
        $this->assertTrue($request->matchPath('#^/foo#'));
    }

    public function testRequestMethodConstraint()
    {
        $request = RouteRequest::createFromGlobals('GET', '/foo/bar');
        $this->assertNotNull($request);
        $this->assertTrue($request->matchRequestMethod('GET'));
        $this->assertFalse($request->matchRequestMethod('POST'));
    }
}

