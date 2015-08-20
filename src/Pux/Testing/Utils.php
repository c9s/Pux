<?php
namespace Pux\Testing;

class Utils
{
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


