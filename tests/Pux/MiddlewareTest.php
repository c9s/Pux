<?php
use Pux\Middleware;
use Pux\RouteRequest;

class TryCatchMiddleware extends Middleware
{
    public function call($env, $res)
    {
        return $res;
    }

}

/**
 * createGlobals helps you define a global object for testing
 */
function createGlobals($method, $requestUri, $pathInfo = '')
{
    return [  
        '_SERVER' => [ 
            'REQUEST_METHOD' => $method,
            'REQUEST_URI' => $requestUri,
            'PATH_INFO' => $pathInfo,
        ],
        '_REQUEST' => [  ],
        '_POST' => [  ],
        '_GET' => [  ],
        '_ENV' => [  ],
        '_COOKIE' => [  ],
    ];
}


class MiddlewareTest extends PHPUnit_Framework_TestCase
{

    public function testMiddleware()
    {
        $app = function(array $environment, array $response) {
            $request = RouteRequest::createFromGlobals($environment['_SERVER']['PATH_INFO'], $environment);
            return $response;
        };

        $middleware = new TryCatchMiddleware($app);

        // $request = RouteRequest::create('GET', '/path');
        $env = createGlobals('GET', '/foo');
        $response = $middleware($env, [200, [], []]);
        $this->assertNotEmpty($response);
    }
}




