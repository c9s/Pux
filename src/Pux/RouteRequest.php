<?php
namespace Pux;
use Pux\RouteRequestMatcher;
use Universal\Http\Request;

/**
 * RouteRequest defines request information for routing.
 *
 * You can use RouteRequest's constraint methods to check if a request matches
 * your logics.
 *
 * TODO: extends from Universal\Http\Request
 */
class RouteRequest implements RouteRequestMatcher
{


    /**
     * @var array $headers
     */
    protected $headers = array();


    
    /**
     * @var string request method
     */
    public $requestMethod;


    /**
     * @var string request path
     */
    public $path;

    /**
     * @var array $server
     */
    public $serverParameters = array();


    /**
     * @var array parameters
     */
    public $parameters = array();



    /**
     * @var array query parameter from $_GET
     */
    public $queryParameters = array();


    /**
     * @var array body parameter from $_POST
     */
    public $bodyParameters = array();


    /**
     * @var array cookie parameters
     */
    public $cookies = array();


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


    public function getPath()
    {
        return $this->path;
    }

    public function getRequestMethod()
    {
        return $this->requestMethod;
    }

    public function getParameters()
    {
        return $this->parameters;
    }

    public function getQueryParameters()
    {
        return $this->queryParameters;
    }

    public function getBodyParameters()
    {
        return $this->bodyParameters;
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

    public function matchQueryString($pattern, & $matches = array())
    {
        return preg_match($pattern, $this->serverParameters['QUERY_STRING'], $matches) !== FALSE;
    }


    public function equalsPort($port)
    {
        if (isset($this->serverParameters['SERVER_PORT'])) {
            return intval($this->serverParameters['SERVER_PORT']) == intval($port);
        }
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
        if (isset($this->serverParameters['HTTP_HOST'])) {
            return strcasecmp($this->serverParameters['HTTP_HOST'], $host) === 0;
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
        if (isset($this->serverParameters['HTTP_HOST'])) {
            return preg_match($host, $this->serverParameters['HTTP_HOST'], $matches) !== FALSE;
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
    public static function createHeadersFromServerGlobal($server)
    {
        $headers = array();
        foreach ($server as $key => $value) { 
            // we only convert the fields that are with HTTP_ prefix
            if (substr($key, 0, 5) == 'HTTP_') { 
                $headers[str_replace(' ', '-', ucwords(strtolower(str_replace('_', ' ', substr($key, 5)))))] = $value; 
            } 
        }
        return $headers;
    }

    /**
     * @param string $path The path argument is separated from environment
     *                     because we might want to use specific path from REQUEST_URI or
     *                     PATH_INFO
     *
     * @param array $environment The environment variable
     */
    public static function createFromEnvironment($path, array $environment, array $queryParameters = array(), array $bodyParameters = array())
    {
        $requestMethod = $environment['REQUEST_METHOD'];
        // $pathInfo = $environment['PATH_INFO'];
    }


    /**
     * A helper function for creating request object based on request method and request uri
     *
     * @param string $method
     * @param string $path
     * @param array $headers The headers will be built on $_SERVER if the argument is null.
     *
     * @return RouteRequest
     */
    public static function create($method, $path, array $headers = null, array $queryParameters = array(), array $bodyParameters = array(), array $cookies = array())
    {
        $request = new self($method, $path);

        if ($headers) {
            $request->headers = $headers;
        } else {
            if (function_exists('getallheaders')) {
                $request->headers = getallheaders();
            } else {
                // TODO: filter array keys by their prefix, consider adding an extension function for this.
                $request->headers = self::createHeadersFromServerGlobal($_SERVER);
            }
        }
        if (isset($_SERVER)) {
            $request->serverParameters = $_SERVER;
        }
        $request->parameters = array_merge_recursive($queryParameters, $bodyParameters);
        $request->queryParameters = $queryParameters;
        $request->bodyParameters = $bodyParameters;
        $request->cookies = $cookies;
        return $request;
    }

    /**
     * Create request object from global variables
     */
    public static function createFromGlobals($path, array $globals)
    {
        $request = new self($globals['_SERVER']['REQUEST_METHOD'], $path);

        if (function_exists('getallheaders')) {
            $request->headers = getallheaders();
        } else {
            // TODO: filter array keys by their prefix, consider adding an extension function for this.
            $request->headers = self::createHeadersFromServerGlobal($globals['_SERVER']);
        }

        $request->serverParameters = $globals['_SERVER'];
        $request->parameters       = $globals['_REQUEST'];
        $request->queryParameters  = $globals['_GET'];
        $request->bodyParameters   = $globals['_POST'];
        $request->cookies          = $globals['_COOKIE'];
        return $request;
    }
}



