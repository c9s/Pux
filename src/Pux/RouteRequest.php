<?php
namespace Pux;
use Pux\RouteRequestMatcher;

class RouteRequest implements RouteRequestMatcher
{

    /**
     * @var array $server
     */
    protected $server = array();

    /**
     * @var array $headers
     */
    protected $headers = array();

    /**
     * @var array parameters
     */
    protected $parameters = array();

    /**
     * @var string request method
     */
    protected $requestMethod;


    /**
     * @var string request path
     */
    protected $path;


    /**
     *
     * @param string $requestMethod
     * @param string $path
     */
    public function __construct($requestMethod, $path)
    {
        $this->requestMethod = $requestMethod;
        $this->path = $path;
    }



    /**
     *
     *
     * @param contraints[]
     */
    public function matchConstraints(array $constraints)
    {
        foreach ($constraints as $constraint) {
            $result = true;
            if (isset($constraints['host_like'])) {
                $result = $result && $this->matchHost($constraints['host_like']);
            }

            if (isset($constraints['host'])) {
                $result = $result && $this->equalsHost($constraints['host']);
            }

            if (isset($constraints['request_method'])) {
                $result = $result && $this->matchRequestMethod($constraints['request_method']);
            }

            if (isset($constraints['path_like'])) {
                $result = $result && $this->matchPath($constraints['path_like']);
            }

            if (isset($constraints['path'])) {
                $result = $result && $this->equalsPath($constraints['path']);
            }

            // If it matches all constraints, we simply return true and skip other constraints
            if ($result) {
                return true;
            }
            // try next one
        }
        return false;
    }


    public function containsPath($path)
    {
        return strpos($this->path, $path) !== FALSE;
    }


    public function matchPathSuffix($suffix)
    {
        $p = strrpos($this->path, $suffix);
        return ($p == strlen($this->path) - strlen($suffix));
    }


    /**
     * Check if the request host is in the list of host.
     *
     * @param array $hosts
     * @return boolean
     */
    public function isOneOfHosts(array $hosts)
    {
        foreach ($hosts as $host) {
            if ($this->matchHost($host)) {
                return true;
            }
        }
        return false;
    }


    public function equalsHost($host)
    {
        if (isset($this->server['HTTP_HOST'])) {
            return strcasecmp($this->server['HTTP_HOST'], $host) === 0;
        }
        return false;
    }

    public function equalsPath($path)
    {
        return strcasecmp($this->path, $path) === 0;
    }


    public function matchPath($pattern, & $matches = array())
    {
        return preg_match($pattern, $this->path, $matches) !== FALSE;
    }

    public function matchHost($host, & $matches = array())
    {
        if (isset($this->server['HTTP_HOST'])) {
            return preg_match($host, $this->server['HTTP_HOST'], $matches) !== FALSE;
        }
        // the HTTP HOST is not defined.
        return false;
    }

    /**
     * matchRequestMethod does not use PCRE pattern to match request method.
     *
     * @param string $requestMethod
     */
    public function matchRequestMethod($requestMethod)
    {
        return strcasecmp($this->requestMethod, $requestMethod) === 0;
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



