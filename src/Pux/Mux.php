<?php
// vim:et:sw=4:ts=4:sts=4:
namespace Pux;
use Pux\PatternCompiler;
use Pux\Controller;
use Closure;
use Exception;
use LogicException;

define('REQUEST_METHOD_GET', 1);
define('REQUEST_METHOD_POST', 2);
define('REQUEST_METHOD_PUT', 3);
define('REQUEST_METHOD_DELETE', 4);
define('REQUEST_METHOD_PATCH', 5);
define('REQUEST_METHOD_HEAD', 6);
define('REQUEST_METHOD_OPTIONS', 7);

use Pux\Dispatcher\PathDispatcher;

class Mux implements PathDispatcher
{
    public $routes = array();

    public $staticRoutes = array();

    public $routesById = array();


    /**
     * @var Mux[id]
     */
    public $submux = array();

    public $id;


    /**
     * @var boolean expand routes to parent mux.
     *
     * When expand is enabled, all mounted Mux will expand the routes to the parent mux.
     * This improves the dispatch performance when you have a lot of sub mux to dispatch.
     *
     * When expand is enabled, the pattern comparison strategy for
     * strings will match the full string.
     *
     * When expand is disabled, the pattern comparison strategy for
     * strings will match the prefix.
     */
    public $expand = false;

    public static $id_counter = 0;


    public static function generate_id() {
        return ++static::$id_counter;
    }

    public function getId() {
        if ( $this->id ) {
            return $this->id;
        }
        return $this->id = self::generate_id();
    }

    public function appendRoute($pattern, $callback, array $options = array())
    {
        $this->routes[] = array( false, $pattern, $callback, $options );
    }

    public function appendPCRERoute(array $routeArgs, $callback)
    {
        $this->routes[] = array(
            true, // PCRE
            $routeArgs['compiled'],
            $callback,
            $routeArgs,
        );
    }

    /**
     * Mount a Mux or a Controller object on a specific path.
     *
     * @param string $pattern
     * @param Mux|Controller $mux
     * @param array $options
     */
    public function mount($pattern, $mux, array $options = array())
    {
        if ($mux instanceof Controller) {

            $mux = $mux->expand();

        } else if ($mux instanceof Closure) {

            // we pass newly created Mux object to the closure to let it initialize it.
            if ($ret = $mux($mux = new Mux)) {
                if ($ret instanceof Mux) {
                    $mux = $ret;
                } else {
                    throw new LogicException("Invalid object returned from Closure.");
                }
            }

        } else if ((!is_object($mux) || !($mux instanceof Mux)) && is_callable($mux)) {
            $mux($mux = new Mux());
        }

        $muxId = $mux->getId();
        $this->add($pattern, $muxId, $options);
        $this->submux[ $muxId ] = $mux;
        /*
        if ($this->expand) {
            $pcre = strpos($pattern,':') !== false;

            // rewrite submux routes
            foreach ($mux->routes as $route) {
                // process for pcre
                if ( $route[0] || $pcre ) {
                    $newPattern = $pattern . ( $route[0] ? $route[3]['pattern'] : $route[1] );
                    $routeArgs = PatternCompiler::compile($newPattern, 
                        array_replace_recursive($options, $route[3]) );
                    $this->appendPCRERoute( $routeArgs, $route[2] );
                } else {
                    $this->routes[] = array(
                        false,
                        $pattern . $route[1],
                        $route[2],
                        isset($route[3]) ? array_replace_recursive($options, $route[3]) : $options,
                    );
                }
            }
        */
    }

    public function delete($pattern, $callback, array $options = array())
    {
        $options['method'] = REQUEST_METHOD_DELETE;
        $this->add($pattern, $callback, $options);
    }

    public function put($pattern, $callback, array $options = array())
    {
        $options['method'] = REQUEST_METHOD_PUT;
        $this->add($pattern, $callback, $options);
    }

    public function get($pattern, $callback, array $options = array())
    {
        $options['method'] = REQUEST_METHOD_GET;
        $this->add($pattern, $callback, $options);
    }

    public function post($pattern, $callback, array $options = array())
    {
        $options['method'] = REQUEST_METHOD_POST;
        $this->add($pattern, $callback, $options);
    }

    public function patch($pattern, $callback, array $options = array())
    {
        $options['method'] = REQUEST_METHOD_PATCH;
        $this->add($pattern, $callback, $options);
    }


    public function head($pattern, $callback, array $options = array())
    {
        $options['method'] = REQUEST_METHOD_HEAD;
        $this->add($pattern, $callback, $options);
    }


    public function options($pattern, $callback, array $options = array())
    {
        $options['method'] = REQUEST_METHOD_OPTIONS;
        $this->add($pattern, $callback, $options);
    }

    public function any($pattern, $callback, array $options = array())
    {
        $this->add($pattern, $callback, $options);
    }

    public function add($pattern, $callback, array $options = array())
    {
        if (is_string($callback) && strpos($callback,':') !== false ) {
            $callback = explode(':', $callback);
        }

        // Convert request method constraint to constant if it's passed by string
        if (isset($options['method']) && is_string($options['method'])) {
            $options['method'] = Mux::getRequestMethodConstant($options['method']);
        }

        // compile place holder to patterns
        $pcre = strpos($pattern,':') !== false;
        if ($pcre) {
            $routeArgs = is_integer($callback)
                ? PatternCompiler::compilePrefix($pattern, $options) 
                : PatternCompiler::compile($pattern, $options)
                ;

            // generate a pcre pattern route
            $route = array( 
                true, // PCRE
                $routeArgs['compiled'],
                $callback,
                $routeArgs,
            );
            if ( isset($options['id']) ) {
                $this->routesById[ $options['id'] ] = $route;
            }
            return $this->routes[] = $route;
        } else {
            $route = array(
                false,
                $pattern,
                $callback,
                $options,
            );
            if ( isset($options['id']) ) {
                $this->routesById[ $options['id'] ] = $route;
            }
            // generate a simple string route.
            return $this->routes[] = $route;
        }
    }

    public function getRoute($id) {
        if ( isset($this->routesById[$id]) ) {
            return $this->routesById[$id];
        }

    }

    public function sort()
    {
        usort($this->routes, array('Pux\\MuxCompiler','sort_routes'));
    }

    static public function sort_routes($a, $b)
    {
        if ( $a[0] && $b[0] ) {
            return strlen($a[3]['compiled']) > strlen($b[3]['compiled']);
        } elseif ( $a[0] ) {
            return 1;
        } elseif ( $b[0] ) {
            return -1;
        }
        if ( strlen($a[1]) > strlen($b[1]) ) {
            return 1;
        } elseif ( strlen($a[1]) == strlen($b[1]) ) {
            return 0;
        } else {
            return -1;
        }
    }


    public function compile($outFile, $sortBeforeCompile = true)
    {
        // compile routes to php file as a cache.
        if ($sortBeforeCompile) {
            $this->sort();
        }

        $code = '<?php return ' . $this->export() . ';';
        return file_put_contents($outFile, $code);
    }

    public function getSubMux($id)
    {
        if ( isset($this->submux[ $id ] ) ) {
            return $this->submux[ $id ];
        }
    }

    public static function getRequestMethodConstant($method)
    {
        switch (strtoupper($method)) {
            case "POST":
                return REQUEST_METHOD_POST;
            case "GET":
                return REQUEST_METHOD_GET;
            case "PUT":
                return REQUEST_METHOD_PUT;
            case "DELETE":
                return REQUEST_METHOD_DELETE;
            case "PATCH":
                return REQUEST_METHOD_PATCH;
            case "HEAD":
                return REQUEST_METHOD_HEAD;
            case "OPTIONS":
                return REQUEST_METHOD_OPTIONS;
            default:
                return 0;
        }
    }


    /**
     * Try to find a matched route.
     *
     * @param string $path
     * @param RouteRequest $request
     *
     */
    public function match($path, RouteRequest $request = null)
    {
        $requestMethod = null;
        if (isset($_SERVER['REQUEST_METHOD'])) {
            $requestMethod = self::getRequestMethodConstant($_SERVER['REQUEST_METHOD']);
        }
        //Allow HTTP method to be overwritten by HTTP_X_HTTP_METHOD_OVERRIDE
        if ('POST' === $requestMethod) {
            $requestMethod = isset($_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE']) ? $_SERVER['HTTP_X_HTTP_METHOD_OVERRIDE'] : $requestMethod;
        }

        foreach ($this->routes as $route) {
            // If the route is using pcre pattern marching...
            if ($route[0]) {
                if (!preg_match($route[1], $path , $matches)) {
                    continue;
                }
                $route[3]['vars'] = $matches;

                // validate request method
                if (isset($route[3]['method']) && $route[3]['method'] != $requestMethod)
                    continue;
                if (isset($route[3]['domain']) && $route[3]['domain'] != $_SERVER["HTTP_HOST"])
                    continue;
                if (isset($route[3]['secure']) && $route[3]['secure'] && $_SERVER["HTTPS"])
                    continue;
                return $route;
            } else {
                // prefix match is used when expanding is not enabled.
                if ((is_int($route[2]) && strncmp($route[1], $path, strlen($route[1]) ) === 0 ) || $route[1] == $path) {
                    // validate request method
                    if ( isset($route[3]['method']) && $route[3]['method'] != $requestMethod )
                        continue;
                    if ( isset($route[3]['domain']) && $route[3]['domain'] != $_SERVER["HTTP_HOST"] )
                        continue;
                    if ( isset($route[3]['secure']) && $route[3]['secure'] && $_SERVER["HTTPS"] )
                        continue;
                    return $route;
                }
                continue;
            }
        }
    }

    public function dispatch($path, $request = null)
    {
        if ($route = $this->match($path)) {
            if (is_integer($route[2])) {
                $submux = $this->submux[$route[2]];


                // sub-path and call submux to dispatch
                // for pcre pattern?
                if ($route[0]) { 
                    $matchedString = $route[3]['vars'][0];
                    return $submux->dispatch(substr($path, strlen($matchedString)));
                } else {
                    $s = substr($path, strlen($route[1]));
                    return $submux->dispatch(
                        substr($path, strlen($route[1])) ?: ''
                    );
                }
            }
            return $route;
        }
    }

    public function length()
    {
        return count($this->routes);
    }

    public function getRoutes()
    {
        return $this->routes;
    }

    public function setRoutes($routes)
    {
        $this->routes = $routes;
    }

    public function export()
    {
        return var_export($this, true);
    }


    /**
     * url method generates the related URL for a route
     *
     * XXX: Untested
     *
     * @see https://github.com/c9s/Pux/issues/4
     *
     * @param string $id route id
     * @param array $params the parameters for an url
     *
     * @return string
     */
    public function url($id, array $params = array())
    {
        $route = $this->getRoute($id);

        if (! isset($route)) {
            throw new \RuntimeException('Named route not found for id: ' . $id);
        }

        $search = array();
        foreach ($params as $key => $value) {
            // try to match ':{key}' fragments and replace it with value
            $search[] = '#:' . preg_quote($key, '#') . '\+?(?!\w)#';
        }

        $pattern = preg_replace($search, $params, $route[3]['pattern']);

        // Remove remnants of unpopulated, trailing optional pattern segments, escaped special characters
        return preg_replace('#\(/?:.+\)|\(|\)|\\\\#', '', $pattern);
    }


    public static function __set_state($array)
    {
        $mux = new self;
        $mux->routes = $array['routes'];
        $mux->submux = $array['submux'];
        if (isset($array['routesById'])) {
            $mux->routesById = $array['routesById'];
        }
        $mux->id = $array['id'];
        return $mux;
    }

}


