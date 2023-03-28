<?php

// vim:et:sw=4:ts=4:sts=4:
namespace Pux;
use Pux\Controller\ExpandableController;
use Pux\Controller\Controller;
use Pux\RouteRequest;
use Pux\Expandable;
use Closure;
use LogicException;
use IteratorAggregate;
use ArrayIterator;

define('REQUEST_METHOD_GET', 1);
define('REQUEST_METHOD_POST', 2);
define('REQUEST_METHOD_PUT', 3);
define('REQUEST_METHOD_DELETE', 4);
define('REQUEST_METHOD_PATCH', 5);
define('REQUEST_METHOD_HEAD', 6);
define('REQUEST_METHOD_OPTIONS', 7);
/**
 * Mux class provides a built-in dispatch method that can dispatch routes,
 * You can define a Dispatcher to customze the dispatch logic on your own.
 *
 */
class Mux implements IteratorAggregate
{

    /**
     * @var array contains route array
     *
     * the route information is stored in the following format:
     *
     * [
     *    [ pcre (boolean), path (string), callback (callable), options (array) ],
     *    ....
     * ]
     *
     * The fields are in public scope because they are needed to be accessed by MuxCompiler.
     *
     * For each route item, you can define an option array for it, but there are some reserved keys:
     *
     *  - 'method': request method constraint
     *  - 'domain': host constraint
     *  - 'secure': https constraint
     *  - 'id'
     *  - 'requirements'
     *  - 'mount_path': the mounted path.
     *
     */
    public $routes = [];

    public $staticRoutes = [];

    public $routesById = [];

    /**
     * @var Mux[id]
     */
    public $submux = [];

    public $id;

    /**
     * @var bool expand routes to parent mux.
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
    public $expand = true;

    /**
     * @param Pux\Mux The parent mux object
     */
    protected $parent;

    public static $id_counter = 0;

    public static function generate_id()
    {
        return ++static::$id_counter;
    }

    public function getId()
    {
        if ($this->id) {
            return $this->id;
        }

        return $this->id = self::generate_id();
    }

    public function appendRouteArray(array $route)
    {
        $this->routes[] = $route;
    }

    public function appendRoute($pattern, $callback, array $options = [])
    {
        $this->routes[] = [false, $pattern, $callback, $options];
    }

    public function appendPCRERoute(array $routeArgs, $callback)
    {
        $this->routes[] = [
            true,
            // PCRE
            $routeArgs['compiled'],
            $callback,
            $routeArgs,
        ];
    }


    public function setParent(Mux $mux)
    {
        $this->parent = $mux;
    }

    /**
     * Mount a Mux or a Controller object on a specific path.
     *
     * @param string         $pattern
     */
    public function mount($pattern, \Pux\Mux|\Pux\Controller\Controller $mux, array $options = [])
    {
        // Save the mount path in options array
        $options['mount_path'] = $pattern;

        if ($mux instanceof Expandable) {

            $mux = $mux->expand($options);

        } else if ($mux instanceof Closure) {

            // we pass the newly created Mux object to the builder closure to initialize routes.
            if ($ret = $mux($mux = new Mux())) {
                if ($ret instanceof Mux) {
                    $mux = $ret;
                } else {
                    throw new LogicException('Invalid object returned from Closure.');
                }
            }
        } elseif ((!is_object($mux) || !($mux instanceof self)) && is_callable($mux)) {
            $mux($mux = new self());
        }

        // Save the constructed mux object in options array, so we can fetch
        // the expanded mux object in controller object later.
        $mux->setParent($this);
        $options['mux'] = $mux;


        if ($this->expand) {
            $pcre = str_contains($pattern,':');

            // rewrite submux routes
            foreach ($mux->routes as $route) {
                // process for pcre
                if ($route[0] || $pcre) {
                    $newPattern = $pattern . ( $route[0] ? $route[3]['pattern'] : $route[1] );
                    $routeArgs = PatternCompiler::compile($newPattern, 
                        array_replace_recursive($options, $route[3]) );
                    $this->appendPCRERoute( $routeArgs, $route[2] );
                } else {
                    $this->routes[] = [false, $pattern . $route[1], $route[2], isset($route[3]) ? array_replace_recursive($options, $route[3]) : $options];
                }
            }
        } else {

            $muxId = $mux->getId();
            $this->add($pattern, $muxId, $options);
            $this->submux[ $muxId ] = $mux;

        }
    }

    public function delete($pattern, $callback, array $options = [])
    {
        $options['method'] = REQUEST_METHOD_DELETE;
        $this->add($pattern, $callback, $options);
    }

    public function put($pattern, $callback, array $options = [])
    {
        $options['method'] = REQUEST_METHOD_PUT;
        $this->add($pattern, $callback, $options);
    }

    public function get($pattern, $callback, array $options = [])
    {
        $options['method'] = REQUEST_METHOD_GET;
        $this->add($pattern, $callback, $options);
    }

    public function post($pattern, $callback, array $options = [])
    {
        $options['method'] = REQUEST_METHOD_POST;
        $this->add($pattern, $callback, $options);
    }

    public function patch($pattern, $callback, array $options = [])
    {
        $options['method'] = REQUEST_METHOD_PATCH;
        $this->add($pattern, $callback, $options);
    }

    public function head($pattern, $callback, array $options = [])
    {
        $options['method'] = REQUEST_METHOD_HEAD;
        $this->add($pattern, $callback, $options);
    }

    public function options($pattern, $callback, array $options = [])
    {
        $options['method'] = REQUEST_METHOD_OPTIONS;
        $this->add($pattern, $callback, $options);
    }

    public function any($pattern, $callback, array $options = [])
    {
        $this->add($pattern, $callback, $options);
    }

    public function add($pattern, $callback, array $options = [])
    {
        if (is_string($callback) && str_contains($callback, ':')) {
            $callback = explode(':', $callback);
        }

        // Convert request method constraint to constant if it's passed by string
        if (isset($options['method']) && is_string($options['method'])) {
            $options['method'] = self::convertRequestMethodConstant($options['method']);
        }

        // compile place holder to patterns
        $pcre = str_contains((string) $pattern, ':');
        if ($pcre) {
            $routeArgs = is_integer($callback)
                ? PatternCompiler::compilePrefix($pattern, $options)
                : PatternCompiler::compile($pattern, $options)
                ;

            // generate a pcre pattern route
            $route = [
                true,
                // PCRE
                $routeArgs['compiled'],
                $callback,
                $routeArgs,
            ];
            if (isset($options['id'])) {
                $this->routesById[ $options['id'] ] = $route;
            }

            return $this->routes[] = $route;
        } else {
            $route = [false, $pattern, $callback, $options];
            if (isset($options['id'])) {
                $this->routesById[ $options['id'] ] = $route;
            }

            // generate a simple string route.
            return $this->routes[] = $route;
        }
    }

    public function getRoute($id)
    {
        if (isset($this->routesById[$id])) {
            return $this->routesById[$id];
        }
    }

    public function sort()
    {
        usort($this->routes, \Pux\MuxCompiler::sort_routes(...));
    }

    public static function sort_routes($a, $b)
    {
        if ($a[0] && $b[0]) {
            return strlen((string) $a[3]['compiled']) > strlen((string) $b[3]['compiled']);
        } elseif ($a[0]) {
            return 1;
        } elseif ($b[0]) {
            return -1;
        }

        if (strlen((string) $a[1]) > strlen((string) $b[1])) {
            return 1;
        } elseif (strlen((string) $a[1]) == strlen((string) $b[1])) {
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

        $code = '<?php return '.$this->export().';';

        return file_put_contents($outFile, $code);
    }

    public function getSubMux($id)
    {
        if (isset($this->submux[ $id ])) {
            return $this->submux[ $id ];
        }
    }

    public static function convertRequestMethodConstant($method)
    {
        return match (strtoupper((string) $method)) {
            'POST' => REQUEST_METHOD_POST,
            'GET' => REQUEST_METHOD_GET,
            'PUT' => REQUEST_METHOD_PUT,
            'DELETE' => REQUEST_METHOD_DELETE,
            'PATCH' => REQUEST_METHOD_PATCH,
            'HEAD' => REQUEST_METHOD_HEAD,
            'OPTIONS' => REQUEST_METHOD_OPTIONS,
            default => 0,
        };
    }

    /**
     * Find a matched route with the path constraint in the current mux object.
     *
     * @param string       $path
     *
     * @return array
     */
    public function match($path, RouteRequest $routeRequest = null)
    {
        $requestMethod = null;

        if ($routeRequest !== null) {

            $requestMethod = self::convertRequestMethodConstant($routeRequest->getRequestMethod());

        } else if (isset($_SERVER['REQUEST_METHOD'])) {

            $requestMethod = self::convertRequestMethodConstant($_SERVER['REQUEST_METHOD']);

        }

        foreach ($this->routes as $route) {
            // If the route is using pcre pattern marching...
            if ($route[0]) {
                if (!preg_match($route[1], $path, $matches)) {
                    continue;
                }

                $route[3]['vars'] = $matches;

                // validate request method
                if (isset($route[3]['method']) && $route[3]['method'] != $requestMethod) {
                    continue;
                }

                if (isset($route[3]['domain']) && $route[3]['domain'] != $_SERVER['HTTP_HOST']) {
                    continue;
                }

                if (isset($route[3]['secure']) && $route[3]['secure'] && $_SERVER['HTTPS']) {
                    continue;
                }

                return $route;
            } else {
                // prefix match is used when expanding is not enabled.
                if ((
                        (is_int($route[2]) || $route[2] instanceof self || $route[2] instanceof \PHPSGI\App)
                        && strncmp((string) $route[1], $path, strlen((string) $route[1])) === 0
                    ) || $route[1] == $path) {
                    // validate request method
                    if (isset($route[3]['method']) && $route[3]['method'] != $requestMethod) {
                        continue;
                    }

                    if (isset($route[3]['domain']) && $route[3]['domain'] != $_SERVER['HTTP_HOST']) {
                        continue;
                    }

                    if (isset($route[3]['secure']) && $route[3]['secure'] && $_SERVER['HTTPS']) {
                        continue;
                    }

                    return $route;
                }

                continue;
            }
        }
    }

    public function dispatchRequest(RouteRequest $routeRequest)
    {
        return $this->dispatch($routeRequest->getPath(), $routeRequest);
    }

    /**
     * Match route in the current Mux and submuxes recursively.
     *
     * The RouteRequest object is used for serving request method, host name
     * and other route information.
     *
     * @param string $path
     * @param Pux\RouteRequest $routeRequest
     *
     * @return array
     */
    public function dispatch($path, RouteRequest $routeRequest = null)
    {
        if ($route = $this->match($path)) {
            // When the callback is an integer, it's refereing to a submux object.
            if (is_integer($route[2])) {
                $submux = $this->submux[$route[2]];

                // sub-path and call submux to dispatch
                // for pcre pattern?
                if ($route[0]) {

                    $matchedString = $route[3]['vars'][0];

                    return $submux->dispatch(substr($path, strlen((string) $matchedString)));

                } else {
                    return $submux->dispatch(
                        substr($path, strlen((string) $route[1])) ?: ''
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

    public function setRoutes(array $routes)
    {
        $this->routes = $routes;
    }

    public function export()
    {
        return var_export($this, true);
    }

    /**
     * url method generates the related URL for a route.
     *
     * XXX: Untested
     *
     * @see https://github.com/c9s/Pux/issues/4
     *
     * @param string $id     route id
     * @param array  $params the parameters for an url
     *
     * @return string
     */
    public function url($id, array $params = [])
    {
        $route = $this->getRoute($id);

        if (!isset($route)) {
            throw new \RuntimeException('Named route not found for id: '.$id);
        }

        $search = [];
        foreach ($params as $key => $value) {
            // try to match ':{key}' fragments and replace it with value
            $search[] = '#:'.preg_quote($key, '#').'\+?(?!\w)#';
        }

        $pattern = preg_replace($search, $params, (string) $route[3]['pattern']);

        // Remove remnants of unpopulated, trailing optional pattern segments, escaped special characters
        return preg_replace('#\(/?:.+\)|\(|\)|\\\\#', '', $pattern);
    }

    public static function __set_state($array)
    {
        $self = new self();
        $self->routes = $array['routes'];
        $self->submux = $array['submux'];
        if (isset($array['routesById'])) {
            $self->routesById = $array['routesById'];
        }

        $self->id = $array['id'];
        return $self;
    }

    #[\ReturnTypeWillChange]
    public function getIterator()
    {
        return new ArrayIterator($this->routes);
    }

}
