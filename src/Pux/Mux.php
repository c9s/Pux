<?php
// vim:et:sw=4:ts=4:sts=4:
namespace Pux;
use Pux\PatternCompiler;
use Exception;

define('REQUEST_METHOD_GET', 1);
define('REQUEST_METHOD_POST', 2);
define('REQUEST_METHOD_PUT', 3);
define('REQUEST_METHOD_DELETE', 4);
define('REQUEST_METHOD_PATCH', 5);
define('REQUEST_METHOD_HEAD', 6);
define('REQUEST_METHOD_OPTIONS', 7);

class Mux
{
    public $routes = array();

    public $staticRoutes = array();

    public $routesById = array();

    public $submux = array();

    public $id;


    /**
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

    public function appendRoute($pattern, $callback, $options = array() ) {
        $this->routes[] = array( false, $pattern, $callback, $options );
    }

    public function appendPCRERoute($routeArgs, $callback) {
        $this->routes[] = array( 
            true, // PCRE
            $routeArgs['compiled'],
            $callback,
            $routeArgs,
        );
    }

    public function mount($pattern, $mux, $options = array())
    {
        if ( $mux instanceof \Pux\Controller ) {
            $mux = $mux->expand();
        }

        if ( $this->expand ) {
            // rewrite submux routes
            foreach( $mux->routes as $route ) {
                // process for pcre
                if ( $route[0] ) {
                    $newPattern = $pattern . $route[3]['pattern'];
                    $routeArgs = PatternCompiler::compile($newPattern, 
                        array_merge_recursive($route[3], $options) );
                    $this->appendPCRERoute( $routeArgs, $route[2] );
                } else {
                    $this->routes[] = array(
                        false,
                        $pattern . $route[1],
                        $route[2],
                        isset($route[3]) ? array_merge($options, $route[3]) : $options,
                    );
                }
            }
        } else {
            $muxId = $mux->getId();
            $this->add($pattern, $muxId, $options);
            $this->submux[ $muxId ] = $mux;
        }
    }

    public function delete($pattern, $callback, $options = array()) 
    {
        $options['method'] = REQUEST_METHOD_DELETE;
        $this->add($pattern, $callback, $options);
    }

    public function put($pattern, $callback, $options = array()) 
    {
        $options['method'] = REQUEST_METHOD_PUT;
        $this->add($pattern, $callback, $options);
    }

    public function get($pattern, $callback, $options = array()) 
    {
        $options['method'] = REQUEST_METHOD_GET;
        $this->add($pattern, $callback, $options);
    }

    public function post($pattern, $callback, $options = array())
    {
        $options['method'] = REQUEST_METHOD_POST;
        $this->add($pattern, $callback, $options);
    }

    public function patch($pattern, $callback, $options = array())
    {
        $options['method'] = REQUEST_METHOD_PATCH;
        $this->add($pattern, $callback, $options);
    }


    public function head($pattern, $callback, $options = array())
    {
        $options['method'] = REQUEST_METHOD_HEAD;
        $this->add($pattern, $callback, $options);
    }


    public function options($pattern, $callback, $options = array())
    {
        $options['method'] = REQUEST_METHOD_OPTIONS;
        $this->add($pattern, $callback, $options);
    }

    public function any($pattern, $callback, $options = array()) {
        $this->add($pattern, $callback, $options);
    }

    public function add($pattern, $callback, $options = array())
    {
        if ( is_string($callback) && strpos($callback,':') !== false ) {
            $callback = explode(':', $callback);
        }

        // compile place holder to patterns
        $pcre = strpos($pattern,':') !== false;
        if ( $pcre ) {
            $routeArgs = PatternCompiler::compile($pattern, $options);

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

    static public function sort_routes($a, $b) {
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


    public static function getRequestMethodConstant($method) {
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

    public function match($path) {
        $reqmethod = self::getRequestMethodConstant(@$_SERVER['REQUEST_METHOD']);

        foreach( $this->routes as $route ) {
            if ( $route[0] ) {
                if ( ! preg_match($route[1], $path , $regs ) ) {
                    continue;
                }
                $route[3]['vars'] = $regs;

                // validate request method
                if ( isset($route[3]['method']) && $route[3]['method'] != $reqmethod )
                    continue;
                if ( isset($route[3]['domain']) && $route[3]['domain'] != $_SERVER["HTTP_HOST"] )
                    continue;
                if ( isset($route[3]['secure']) && $route[3]['secure'] && $_SERVER["HTTPS"] )
                    continue;
                return $route;
            } else {
                // prefix match is used when expanding is not enabled.
                if ( ( is_int($route[2]) && strncmp($route[1], $path, strlen($route[1]) ) === 0 ) || $route[1] == $path ) {
                    // validate request method
                    if ( isset($route[3]['method']) && $route[3]['method'] != $reqmethod )
                        continue;
                    if ( isset($route[3]['domain']) && $route[3]['domain'] != $_SERVER["HTTP_HOST"] )
                        continue;
                    if ( isset($route[3]['secure']) && $route[3]['secure'] && $_SERVER["HTTPS"] )
                        continue;
                    return $route;
                } else {
                    continue;
                }
            }
        }
    }

    public function dispatch($path) {
        if ( $route = $this->match($path) ) {
            if ( is_int($route[2]) ) {
                $submux = $this->submux[ $route[2] ];

                // sub-path and call submux to dispatch
                // for pcre pattern?
                if ($route[0]) { 
                    $matchedString = $route[3]['vars'][0];
                    return $submux->dispatch( substr($path, strlen($matchedString)) );
                } else {
                    $s = substr($path, strlen($route[1]));
                    return $submux->dispatch(
                        substr($path, strlen($route[1])) ?: '' 
                    );
                }
            } else {
                return $route;
            }
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

    public function export() {
        return var_export($this, true);
    }

    public static function __set_state($array) {
        $mux = new self;
        $mux->routes = $array['routes'];
        $mux->submux = $array['submux'];
        $mux->expand = $array['expand'];
        if ( isset($array['routesById']) ) {
            $mux->routesById = $array['routesById'];
        }
        $mux->id = $array['id'];
        return $mux;
    }

}


