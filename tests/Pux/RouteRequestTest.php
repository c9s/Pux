<?php
use Pux\RouteRequest;

class RouteRequestTest extends PHPUnit_Framework_TestCase
{
    public function testCreateFromGlobals()
    {
        $request = RouteRequest::create('GET', '/foo/bar');
        $this->assertNotNull($request);
    }

    public function testRequestUrl()
    {
        $request = RouteRequest::create('GET', '/foo/bar');
        $this->assertNotNull($request);
        $this->assertTrue($request->pathLike('/foo'));
    }

    public function testRequestMethodConstraint()
    {
        $request = RouteRequest::create('GET', '/foo/bar');
        $this->assertNotNull($request);
        $this->assertTrue($request->requestMethodEqual('GET'));
        $this->assertFalse($request->requestMethodEqual('POST'));
    }

    public function testPathContains()
    {
        $request = RouteRequest::create('GET', '/foo/bar');
        $this->assertNotNull($request);
        $this->assertTrue( $request->pathContain('/foo') );
        $this->assertTrue( $request->pathContain('/bar') );
    }

    public function testMatchPathSuffix()
    {
        $request = RouteRequest::create('GET', '/foo/bar.json');
        $this->assertNotNull($request);
        $this->assertTrue( $request->pathEndWith('.json') );
        $this->assertFalse( $request->pathEndWith('.js') );
        $this->assertFalse( $request->pathEndWith('.xml') );
    }




}

