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
    protected $headers = [];

    /**
     * @param string $requestMethod
     * @param string $path
     */
    public function __construct(public $requestMethod, public $path)
    {
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
        return $this->path ?: '/';
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
    public function matchConstraints(array $constraints): bool
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

    public function queryStringMatch($pattern, array &$matches = [])
    {
        return preg_match($pattern, (string) $this->serverParameters['QUERY_STRING'], $matches) !== false;
    }

    public function portEqual($port)
    {
        if (isset($this->serverParameters['SERVER_PORT'])) {
            return (int) $this->serverParameters['SERVER_PORT'] === (int) $port;
        }
    }

    /**
     * Check if the request host is in the list of host.
     *
     *
     */
    public function isOneOfHosts(array $hosts): bool
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
        $pattern = '#'.preg_quote((string) $path, '#').'#i';

        return preg_match($pattern, $this->path) !== false;
    }

    public function pathMatch($pattern, array &$matches = [])
    {
        return preg_match($pattern, $this->path, $matches) !== false;
    }

    public function pathEqual($path)
    {
        return strcasecmp((string) $path, $this->path) === 0;
    }

    public function pathContain($path): bool
    {
        return str_contains($this->path, (string) $path);
    }

    public function pathStartWith($path): bool
    {
        return str_starts_with($this->path, (string) $path);
    }

    public function pathEndWith($suffix)
    {
        $p = strrpos($this->path, (string) $suffix);

        return ($p == strlen($this->path) - strlen((string) $suffix));
    }

    public function hostMatch($host, array &$matches = [])
    {
        if (isset($this->serverParameters['HTTP_HOST'])) {
            return preg_match($host, (string) $this->serverParameters['HTTP_HOST'], $matches) !== false;
        }

        // the HTTP HOST is not defined.
        return false;
    }

    public function hostEqual($host)
    {
        if (isset($this->serverParameters['HTTP_HOST'])) {
            return strcasecmp((string) $this->serverParameters['HTTP_HOST'], (string) $host) === 0;
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
    public static function create($method, $path, array $env = [])
    {
        $self = new self($method, $path);
        // TODO: filter array keys by their prefix, consider adding an extension function for this.
        $self->headers = function_exists('getallheaders') ? getallheaders() : self::createHeadersFromServerGlobal($env);
        $self->serverParameters = $env['_SERVER'] ?? $env;

        $self->parameters = $env['_REQUEST'] ?? [];
        $self->queryParameters = $env['_GET'] ?? [];
        $self->bodyParameters = $env['_POST'] ?? [];
        $self->cookieParameters = $env['_COOKIE'] ?? [];
        $self->sessionParameters = $env['_SESSION'] ?? [];
        return $self;
    }

    /**
     * Create request object from global variables.
     *
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
        } elseif (isset($env['_SERVER']['REQUEST_METHOD'])) {
            // compatibility for superglobal
            // compatibility for superglobal
            // compatibility for superglobal
            $requestMethod = $env['_SERVER']['REQUEST_METHOD'];
        }

        // create request object with request method and path,
        // we can assign other parameters later.
        $self = new self($requestMethod, $path);

        // TODO: filter array keys by their prefix, consider adding an extension function for this.
        $self->headers = function_exists('getallheaders') ? getallheaders() : self::createHeadersFromServerGlobal($env);
        $self->serverParameters = $env['_SERVER'] ?? $env;

        $self->parameters = $env['_REQUEST'];
        $self->queryParameters = $env['_GET'];
        $self->bodyParameters = $env['_POST'];
        $self->cookieParameters = $env['_COOKIE'];
        $self->sessionParameters = $env['_SESSION'];
        return $env['__request_object'] = $self;
    }
}
