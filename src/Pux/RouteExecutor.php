<?php

namespace Pux;

use Exception;
use LogicException;
use ReflectionClass;
use Closure;

class RouteExecutor
{

    /**
     * @var boolean 
     *
     * Enable executor to use reflection methods to validate controller action
     *              parameter prototype.
     */
    protected $validateControllerAction = false;

    public function __construct()
    {

    }


    /**
     * Execute the matched route.
     *
     * $route: {pcre flag}, {pattern}, {callback}, {options}
     *
     * *callback*:
     *
     * The callback argument can be an array that contains a class name and a method name to be called.
     * a closure object or a function name.
     *
     * *options*:
     *
     * 'constructor_args': arguments for constructing controller object.
     *
     * @return string the response
     */
    public static function execute(array $route, array $environment = array())
    {
        list($pcre, $pattern, $cb, $options) = $route;

        if ($cb instanceof Closure) {
            return $cb($environment);
        }


        // Start handing controller dispatch from here

        // create the reflection class
        $rc = new ReflectionClass($cb[0]);

        // If users define the constructor arguments in options array.
        $constructArgs = null;
        if (isset($options['constructor_args'])) {
            $constructArgs = $options['constructor_args'];
        }

        // If the first argument is a class name string,
        // then create the controller object.
        if (is_string($cb[0])) {
            if (is_a($cb[0],'Pux\\Controller')) {

                $cb[0] = $controller = $rc->newInstanceArgs([$environment]);

            } else {
                $cb[0] = $controller = $constructArgs 
                    ? $rc->newInstanceArgs($constructArgs) 
                    : $rc->newInstance();
            }
        } else if (is_object($cb[0])) {
            // If it's a dynamic controller object
            $controller = $cb[0];
        } else {
            // unsupported callable type
        }

        // check controller action method
        if ($controller && !method_exists($controller, $cb[1])) {
            throw new LogicException("Controller action method '{$cb[1]}' doesn't exist.");
            /*
            throw new Exception('Method ' .
                get_class($controller) . "->{$cb[1]} does not exist.", $route );
            */
        }

        $reflParameters = $rc->getMethod($cb[1])->getParameters();
        $vars = isset($options['vars'])
                ? $options['vars']
                : array()
                ;

        $arguments = array();
        foreach ($reflParameters as $reflParameter) {
            $n = $reflParameter->getName();
            if (isset($vars[ $n ])) {
                $arguments[] = $vars[ $n ];
            } elseif (isset($route[3]['default'][ $n ])
                            && $default = $route[3]['default'][ $n ]) {
                $arguments[] = $default;
            } elseif (!$reflParameter->isOptional() && !$reflParameter->allowsNull()) {
                throw new Exception('parameter is not defined.');
            }
        }

        return call_user_func_array($cb, $arguments);
    }
}
