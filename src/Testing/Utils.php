<?php
namespace Pux\Testing;

class Utils
{

    static public function createEnv($method, $pathInfo)
    {
        $env = [
            'REQUEST_METHOD' => $method,
            'PATH_INFO'    => $pathInfo,
        ];
        $env['_POST']    = [];
        $env['_REQUEST'] = [];
        $env['_GET']     = [];
        $env['_COOKIE']     = [];
        $env['_SESSION']     = [];
        // fallback (backware compatible for $GLOBALS)
        $env['_SERVER']     = [];
        return $env;
    }


    static public function createEnvFromGlobals(array $globals)
    {
        $env = $globals['_SERVER'];
        $env['_REQUEST'] = $globals['_REQUEST'];
        $env['pux.parameters'] = $globals['_REQUEST'];
        $env['_POST'] = $globals['_POST'];
        $env['pux.body_parameters'] = $globals['_POST'];
        $env['_GET'] = $globals['_GET'];
        $env['pux.query_parameters'] = $globals['_GET'];
        $env['_COOKIE'] = $globals['_COOKIE'];
        $env['pux.cookies'] = $globals['_COOKIE'];
        $env['_SESSION'] = $globals['_SESSION'];
        $env['pux.session'] = $globals['_SESSION'];
        return $env;
    }

    /**
    * createGlobals helps you define a global object for testing
    */
    static public function createGlobals($method, $pathInfo)
    {
        return [  
            '_SERVER' => [ 
                'REQUEST_METHOD' => $method,
                'PATH_INFO' => $pathInfo,
            ],
            '_REQUEST' => [  ],
            '_POST' => [  ],
            '_GET' => [  ],
            '_ENV' => [  ],
            '_COOKIE' => [],
            '_SESSION' => [],
        ];
    }
}


