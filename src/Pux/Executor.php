<?php
namespace Pux;
use Exception;
use ReflectionClass;
use ReflectionObject;
use ReflectionParameter;

class Executor
{
    /*
     * $route: {pcre flag}, {pattern}, {callback}, {options}
     */
    public static function execute($route)
    {
        list($pcre,$pattern,$cb,$options) = $route;

        // create the reflection class
        $rc = new ReflectionClass( $cb[0] );

        $constructArgs = null;
        if ( isset($options['constructor_args']) ) {
            $constructArgs = $options['constructor_args'];
        }

        // if the first argument is a class name string,
        // then create the controller object.
        if( is_string($cb[0]) ) {
            $cb[0] = $controller = $constructArgs ? $rc->newInstanceArgs($constructArgs) : $rc->newInstance();
        } else {
            $controller = $cb[0];
        }

        // check controller action method
        if( $controller && ! method_exists( $controller ,$cb[1]) ) {
            throw new Exception('Controller exception');
            /*
            throw new Exception('Method ' .
                get_class($controller) . "->{$cb[1]} does not exist.", $route );
             */
        }

        $rps = $rc->getMethod($cb[1])->getParameters();

        // XXX:

        $vars = isset($options['vars'])
                ? $options['vars']
                : array()
                ;

        $arguments = array();
        foreach( $rps as $param ) {
            $n = $param->getName();
            if( isset( $vars[ $n ] ) )
            {
                $arguments[] = $vars[ $n ];
            }
            else if( isset($route[3]['default'][ $n ] )
                            && $default = $route[3]['default'][ $n ] )
            {
                $arguments[] = $default;
            }
            else if( ! $param->isOptional() && ! $param->allowsNull() ) {
                throw new Exception('parameter is not defined.');
            }
        }
        return call_user_func_array($cb, $arguments);
    }
}

