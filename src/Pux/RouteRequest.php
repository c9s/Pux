<?php
namespace Pux;

class Request
{
    public $server = array();

    public $headers = array();

    public $parameters = array();

    public function __construct()
    {

    }


    public static function createHeadersFromServerGlobal()
    {
        $headers = array();
        foreach ($_SERVER as $key => $value) { 
            // we only convert the fields that are with HTTP_ prefix
            if (substr($name, 0, 5) == 'HTTP_') { 
                $headers[str_replace(' ', '-', ucwords(strtolower(str_replace('_', ' ', substr($name, 5)))))] = $value; 
            } 
        }
        return $headers;
    }


    public static function createFromGlobals()
    {
        $request = new self;
        $request->server = $_SERVER;

        if (function_exists('getallheaders')) {
            $request->headers = getallheaders();
        } else {
            // TODO: filter array keys by their prefix, consider adding an extension function for this.
            $request->headers = self::createHeadersFromServerGlobal();
        }

        $request->parameters = $_REQUEST;
        return $request;
    }
}



