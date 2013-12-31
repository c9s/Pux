<?php
namespace Phux;
use Phux\RouteCompiler;

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
                if ( $path === $route[2] ) {
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


