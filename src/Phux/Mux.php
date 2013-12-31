<?php
namespace Phux;
use Phux\RouteCompiler;
use Exception;

class Mux
{
    public $routes = array();

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

    public function dispatch($path) {
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
                if ( $path === $route[1] ) {
                    return $route;
                } else {
                    continue;
                }
            }

        }
    }

    public function getRoutes() {
        return $this->routes;
    }

}


