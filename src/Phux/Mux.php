<?php
namespace Phux;
use Phux\RouteCompiler;
use Exception;

class Mux
{
    public $routes = array();

    public $subMux = array();

    public static $id_counter = 0;

    public $id;

    public static function generate_id() {
        return ++static::$id_counter;
    }

    public function getId() {
        if ( $this->id ) {
            return $this->id;
        }
        return $this->id = self::generate_id();
    }

    public function mount($pattern, $mux, $options = array())
    {
        $muxId = $mux->getId();
        $this->add($pattern, $mux->getId(), $options);
        $this->subMux[ $muxId ] = $mux;
    }

    public function add($pattern, $callback, $options = array())
    {
        // compile place holder to patterns
        $pcre = strpos($pattern,':') !== false;
        if ( $pcre ) {
            $route = RouteCompiler::compilePattern($pattern, $options);
            $route['compiled'] = sprintf("#^%s$#xs", $route['regex']);

            // generate a pcre pattern route
            return $this->routes[] = array( 
                true, // PCRE
                $route['compiled'],
                $callback,
                $route,
            );
        } else {
            // generate a simple string route.
            return $this->routes[] = array(
                false,
                $pattern,
                $callback,
                $options,
            );
        }
    }


    /**
     * validate controller classes and controller methods before compiling to 
     * route cache.
     */
    public function validate() 
    {
        foreach( $this->routes as $route ) {
            $callback = $route[2];
            if ( is_array($callback) ) {
                $class = $callback[0];
                $method = $callback[1];
                if ( ! class_exists($class, true) ) {
                    throw new Exception("Controller {$class} does not exist.");
                }
                // rebless a controller (extract this to common method)
                $controller = new $class;
                if ( ! method_exists($controller, $method) ) {
                    throw new Exception("Method $method not found in controller $class.");
                }
            }
        }
    }

    public function compile($outFile)
    {
        // compile routes to php file as a cache.

    }

    public function matchRoute($path) {
        $path = rtrim($path, '/');
        foreach( $this->routes as $route ) {
            if ( $route[0] ) {
                if ( preg_match($route[1], $path , $regs ) ) {
                    $route[3]['vars'] = $regs;
                    return $route;
                } else {
                    continue;
                }
            } else {
                if ( strncmp($path, $route[1], strlen($route[1]) ) === 0 ) {
                    return $route;
                } else {
                    continue;
                }
            }
        }
    }

    public function dispatch($path) {
        if ( $route = $this->matchRoute($path) ) {
            if ( is_int($route[2]) ) {
                $subMux = $this->subMux[ $route[2] ];

                // sub path and call subMux to dispatch
                // for pcre pattern?
                if ($route[0]) { 
                    $matchedString = $route[3]['vars'][0];
                    $subpath = substr($path, strlen($matchedString));
                    return $subMux->dispatch($subpath);
                } else {
                    $subpath = substr($path, strlen($route[1]));
                    return $subMux->dispatch($subpath);
                }
            } else {
                return $route;
            }
        }
    }

    public function getRoutes() {
        return $this->routes;
    }

    public function export() {
        return var_export($this, true);
    }

    public static function __set_state($array) {
        $mux = new self;
        $mux->routes = $array['routes'];
        $mux->subMux = $array['subMux'];
        $mux->id = $array['id'];
        return $mux;
    }

}


