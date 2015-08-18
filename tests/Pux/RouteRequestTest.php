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

    public function testContainsPath()
    {
        $request = RouteRequest::createFromGlobals('GET', '/foo/bar');
        $this->assertNotNull($request);
        $this->assertTrue( $request->containsPath('/foo') );
        $this->assertTrue( $request->containsPath('/bar') );
    }

    public function testMatchPathSuffix()
    {
        $request = RouteRequest::createFromGlobals('GET', '/foo/bar.json');
        $this->assertNotNull($request);
        $this->assertTrue( $request->matchPathSuffix('.json') );
        $this->assertFalse( $request->matchPathSuffix('.js') );
        $this->assertFalse( $request->matchPathSuffix('.xml') );
    }




}

