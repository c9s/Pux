<?php
namespace Pux;
use Pux\RouteRequestMatcher;

class RouteRequest implements RouteRequestMatcher
{
    protected $server = array();

    protected $headers = array();

    /**
     * @var array parameters
     */
    protected $parameters = array();

    /**
     * @var string request method
     */
    protected $method;


    /**
     * @var string request path
     */
    protected $path;

    public function __construct($method, $path)
    {
        $this->method = $method;
        $this->path = $path;
    }



    /**
     * Converts global $_SERVER variables to header values.
     *
     * @return array
     */
    public static function createHeadersFromServerGlobal()
    {
        $headers = array();
        foreach ($_SERVER as $key => $value) { 
            // we only convert the fields that are with HTTP_ prefix
            if (substr($key, 0, 5) == 'HTTP_') { 
                $headers[str_replace(' ', '-', ucwords(strtolower(str_replace('_', ' ', substr($key, 5)))))] = $value; 
            } 
        }
        return $headers;
    }


    /**
     * Create global objects
     */
    public static function createFromGlobals($method, $path)
    {
        $request = new self($method, $path);
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



