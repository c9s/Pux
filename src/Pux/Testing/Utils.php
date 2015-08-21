<?php
namespace Pux\Testing;

class Utils
{

    static public function createEnv($method, $requestUri)
    {
        $env = [
            'REQUEST_METHOD' => $method,
            'REQUEST_URI'    => $requestUri,
        ];
        $env['_POST']    = array();
        $env['_REQUEST'] = array();
        $env['_GET']     = array();
        $env['_COOKIE']     = array();
        return $env;
    }


    static public function createEnvFromGlobals($method, $requestUri, $pathInfo)
    {
        $env = $globals['_SERVER'];
        $env['_REQUEST'] = $env['pux.parameters']       = $globals['_REQUEST'];
        $env['_POST']    = $env['pux.body_parameters']  = $globals['_POST'];
        $env['_GET']     = $env['pux.query_parameters'] = $globals['_GET'];
        $env['_COOKIE']  = $env['pux.cookies'] = $globals['_COOKIE'];
        return $env;
    }

    /**
    * createGlobals helps you define a global object for testing
    */
    static public function createGlobals($method, $requestUri, $pathInfo = '')
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
}


