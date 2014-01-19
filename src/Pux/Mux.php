<?php
namespace Pux;
use Pux\PatternCompiler;
use Exception;

define('REQ_METHOD_GET', 1);
define('REQ_METHOD_POST', 2);
define('REQ_METHOD_PUT', 3);
define('REQ_METHOD_DELETE', 4);

class Mux
{
    public $routes = array();

    public $staticRoutes = array();

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
                        $options,
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
        $options['method'] = REQ_METHOD_DELETE;
        $this->add($pattern, $callback, $options);
    }

    public function put($pattern, $callback, $options = array()) 
    {
        $options['method'] = REQ_METHOD_PUT;
        $this->add($pattern, $callback, $options);
    }

    public function get($pattern, $callback, $options = array()) 
    {
        $options['method'] = REQ_METHOD_GET;
        $this->add($pattern, $callback, $options);
    }

    public function post($pattern, $callback, $options = array()) {
        $options['method'] = REQ_METHOD_POST;
        $this->add($pattern, $callback, $options);
    }

    public function add($pattern, $callback, $options = array())
    {
        // compile place holder to patterns
        $pcre = strpos($pattern,':') !== false;
        if ( $pcre ) {
            $routeArgs = PatternCompiler::compile($pattern, $options);

            // generate a pcre pattern route
            return $this->routes[] = array( 
                true, // PCRE
                $routeArgs['compiled'],
                $callback,
                $routeArgs,
            );
        } else {
            $route = array(
                false,
                $pattern,
                $callback,
                $options,
            );
            if ( empty($options) ) {
                $this->staticRoutes[$pattern] = $route;
            }
            // generate a simple string route.
            return $this->routes[] = $route;
        }
    }

    public function sort() 
    {
        usort($this->routes, ['Pux\\MuxCompiler','sort_routes']);
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


    public function compile($outFile)
    {
        // compile routes to php file as a cache.
        $this->sort();

        $code = '<?php return ' . $this->export() . ';';
        return file_put_contents($outFile, $code);
    }

    public function getSubMux($id)
    {
        if ( isset($this->submux[ $id ] ) ) {
            return $this->submux[ $id ];
        }
    }


    public function getRequestMethodConstant($method) {
        switch ($method) {
        case "POST":
            return REQ_METHOD_POST;
        case "GET":
            return REQ_METHOD_GET;
        case "PUT":
            return REQ_METHOD_PUT;
        case "DELETE":
            return REQ_METHOD_DELETE;
        default:
            return 0;
        }
    }

    public function match($path) {
        if ( isset($this->staticRoutes[$path]) ) {
            return $this->staticRoutes[$path];
        }
        $reqmethod = $this->getRequestMethodConstant(@$_SERVER['REQUEST_METHOD']);
        foreach( $this->routes as $route ) {
            // validate request method
            if ( isset($route[3]['method']) && $route[3]['method'] != $reqmethod ) {
                continue;
            }

            if ( $route[0] ) {
                if ( preg_match($route[1], $path , $regs ) ) {
                    $route[3]['vars'] = $regs;
                    return $route;
                } else {
                    continue;
                }
            } else {
                if ( $this->expand ) {
                    if ( $path === $route[1] ) {
                        return $route;
                    }
                } else {
                    if ( strncmp($route[1] , $path, strlen($route[1]) ) === 0 ) {
                        return $route;
                    } else {
                        continue;
                    }
                }

            }
        }
    }

    public function dispatch($path) {
        // $path = rtrim($path, '/');
        if ( $route = $this->match($path) ) {
            if ( is_int($route[2]) ) {
                $submux = $this->submux[ $route[2] ];

                // sub path and call submux to dispatch
                // for pcre pattern?
                if ($route[0]) { 
                    $matchedString = $route[3]['vars'][0];
                    return $submux->dispatch( 
                        substr($path, strlen($matchedString))
                    );
                } else {
                    return $submux->dispatch(
                        substr($path, strlen($route[1]))
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
        $mux->id = $array['id'];
        return $mux;
    }

}


