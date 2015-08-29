<?php

namespace Pux;

use Universal\Http\HttpRequest;

/**
 * RouteRequest defines request information for routing.
 *
 * You can use RouteRequest's constraint methods to check if a request matches
 * your logics.
 *
 */
class RouteRequest extends HttpRequest implements RouteRequestMatcher
{
    /**
     * @var array
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
     * @param string $requestMethod
     * @param string $path
     */
    public function __construct($requestMethod, $path)
    {
        $this->requestMethod = $requestMethod;
        $this->path = $path;

        // Note: It's not neccessary to call parent::__construct because we
        // don't depend on superglobal variables.
    }

    /**
     * Return the current path for route dispatching.
     * the path can be PATH_INFO or REQUEST_URI (depends on what the user gives)
     *
     * @return string path
     */
    public function getPath()
    {
        return $this->path;
    }

    /**
     * Return the request method
     *
     * @return string request method.
     */
    public function getRequestMethod()
    {
        return $this->requestMethod;
    }

    /**
     * @param contraints[]
     *
     * @return boolean true on match
     */
    public function matchConstraints(array $constraints)
    {
        foreach ($constraints as $constraint) {
            $result = true;
            if (isset($constraints['host_match'])) {
                $result = $result && $this->hostMatch($constraints['host_match']);
            }

            if (isset($constraints['host'])) {
                $result = $result && $this->hostEqual($constraints['host']);
            }

            if (isset($constraints['request_method'])) {
                $result = $result && $this->requestMethodEqual($constraints['request_method']);
            }

            if (isset($constraints['path_match'])) {
                $result = $result && $this->pathMatch($constraints['path_match']);
            }

            if (isset($constraints['path'])) {
                $result = $result && $this->pathEqual($constraints['path']);
            }

            // If it matches all constraints, we simply return true and skip other constraints
            if ($result) {
                return true;
            }
            // try next one
        }

        return false;
    }

    public function queryStringMatch($pattern, array &$matches = array())
    {
        return preg_match($pattern, $this->serverParameters['QUERY_STRING'], $matches) !== false;
    }

    public function portEqual($port)
    {
        if (isset($this->serverParameters['SERVER_PORT'])) {
            return intval($this->serverParameters['SERVER_PORT']) == intval($port);
        }
    }

    /**
     * Check if the request host is in the list of host.
     *
     * @param array $hosts
     *
     * @return bool
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

    public function pathLike($path)
    {
        $pattern = '#'.preg_quote($path, '#').'#i';

        return preg_match($pattern, $this->path) !== false;
    }

    public function pathMatch($pattern, array &$matches = array())
    {
        return preg_match($pattern, $this->path, $matches) !== false;
    }

    public function pathEqual($path)
    {
        return strcasecmp($path, $this->path) === 0;
    }

    public function pathContain($path)
    {
        return strpos($this->path, $path) !== false;
    }

    public function pathStartWith($path)
    {
        return strpos($this->path, $path) === 0;
    }

    public function pathEndWith($suffix)
    {
        $p = strrpos($this->path, $suffix);

        return ($p == strlen($this->path) - strlen($suffix));
    }

    public function hostMatch($host, array &$matches = array())
    {
        if (isset($this->serverParameters['HTTP_HOST'])) {
            return preg_match($host, $this->serverParameters['HTTP_HOST'], $matches) !== false;
        }
        // the HTTP HOST is not defined.
        return false;
    }

    public function hostEqual($host)
    {
        if (isset($this->serverParameters['HTTP_HOST'])) {
            return strcasecmp($this->serverParameters['HTTP_HOST'], $host) === 0;
        }

        return false;
    }

    /**
     * requestMethodEqual does not use PCRE pattern to match request method.
     *
     * @param string $requestMethod
     */
    public function requestMethodEqual($requestMethod)
    {
        return strcasecmp($this->requestMethod, $requestMethod) === 0;
    }

    /**
     * A helper function for creating request object based on request method and request uri.
     *
     * @param string $method
     * @param string $path
     * @param array  $headers The headers will be built on $_SERVER if the argument is null.
     *
     * @return RouteRequest
     */
    public static function create($method, $path, array $env = array())
    {
        $request = new self($method, $path);

        if (function_exists('getallheaders')) {
            $request->headers = getallheaders();
        } else {
            // TODO: filter array keys by their prefix, consider adding an extension function for this.
            $request->headers = self::createHeadersFromServerGlobal($env);
        }

        if (isset($env['_SERVER'])) {
            $request->serverParameters = $env['_SERVER'];
        } else {
            $request->serverParameters = $env;
        }
        $request->parameters = isset($env['_REQUEST']) ? $env['_REQUEST'] : array();
        $request->queryParameters = isset($env['_GET']) ? $env['_GET'] : array();
        $request->bodyParameters = isset($env['_POST']) ? $env['_POST'] : array();
        $request->cookieParameters = isset($env['_COOKIE']) ? $env['_COOKIE'] : array();
        $request->sessionParameters = isset($env['_SESSION']) ? $env['_SESSION'] : array();

        return $request;
    }

    /**
     * Create request object from global variables.
     *
     * @param array $env
     * @return RouteRequest
     */
    public static function createFromEnv(array $env)
    {
        // cache
        if (isset($env['__request_object'])) {
            return $env['__request_object'];
        }

        if (isset($env['PATH_INFO'])) {
            $path = $env['PATH_INFO'];
        } elseif (isset($env['REQUEST_URI'])) {
            $path = $env['REQUEST_URI'];
        } elseif (isset($env['_SERVER']['PATH_INFO'])) {
            $path = $env['_SERVER']['PATH_INFO'];
        } elseif (isset($env['_SERVER']['REQUEST_URI'])) {
            $path = $env['_SERVER']['REQUEST_URI'];
        } else {
            // XXX: check path or throw exception
            $path = '/';
        }

        $requestMethod = 'GET';
        if (isset($env['REQUEST_METHOD'])) {
            $requestMethod = $env['REQUEST_METHOD'];
        } else if (isset($env['_SERVER']['REQUEST_METHOD'])) { // compatibility for superglobal
            $requestMethod = $env['_SERVER']['REQUEST_METHOD'];
        }

        // create request object with request method and path,
        // we can assign other parameters later.
        $request = new self($requestMethod, $path);
        if (function_exists('getallheaders')) {
            $request->headers = getallheaders();
        } else {
            // TODO: filter array keys by their prefix, consider adding an extension function for this.
            $request->headers = self::createHeadersFromServerGlobal($env);
        }

        if (isset($env['_SERVER'])) {
            $request->serverParameters = $env['_SERVER'];
        } else {
            $request->serverParameters = $env;
        }
        $request->parameters = $env['_REQUEST'];
        $request->queryParameters = $env['_GET'];
        $request->bodyParameters = $env['_POST'];
        $request->cookieParameters = $env['_COOKIE'];
        $request->sessionParameters = $env['_SESSION'];

        return $env['__request_object'] = $request;
    }
}
