<?php

namespace Pux;

use Exception;
use LogicException;
use ReflectionClass;
use Closure;
use Pux\Controller\Controller;

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
    public static function execute(array $route, array $environment = array(), array $response = array())
    {
        list($pcre, $pattern, $callback, $options) = $route;

        if ($callback instanceof Closure) {
            return $callback($environment, $response, $route);
        }


        // Start handing controller dispatch from here

        // create the reflection class
        $rc = new ReflectionClass($callback[0]);

        // If users define the constructor arguments in options array.
        $constructArgs = [];
        if (isset($options['constructor_args'])) {
            $constructArgs = $options['constructor_args'];
        }

        // die('Pux\\Controller\\Controller');

        // If the first argument is a class name string,
        // then create the controller object.
        if (is_string($callback[0])) {


            // If the receiver is a Pux controller, we then know how to pass data to the constructor
            if (is_a($callback[0],'Pux\\Controller\\Controller', true)) {


                // Pux controller accepts ($environment, $matchedRoute) by default
                // But users may define a controller that accepts
                // ($environment, $matchedRoute, ... extra constructor
                // arguments)
                array_unshift($constructArgs, $route);
                array_unshift($constructArgs, $response);
                array_unshift($constructArgs, $environment);

                $callback[0] = $controller = $rc->newInstanceArgs($constructArgs);

                // The init method will only be called when an action is going to be executed.
                $controller->init();

            } else {

                $callback[0] = $controller = !empty($constructArgs)
                                                ? $rc->newInstanceArgs($constructArgs) 
                                                : $rc->newInstance();
            }
        } else if (is_object($callback[0])) {

            // If it's a dynamic controller object
            $controller = $callback[0];

        } else {

            throw new LogicException('Unsupported callback type');

        }

        // check controller action method
        if ($controller && !method_exists($controller, $callback[1])) {
            throw new LogicException("Controller action method '{$callback[1]}' doesn't exist.");
            /*
            throw new Exception('Method ' .
                get_class($controller) . "->{$callback[1]} does not exist.", $route );
            */
        }

        $reflParameters = $rc->getMethod($callback[1])->getParameters();
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

        $return = call_user_func_array($callback, $arguments);
        if (is_array($return)) {
            return $return;
        } else if ($controller instanceof Controller && is_string($return)) {
            $response = $controller->getResponse() ?: array();
            if (!isset($response[0])) {
                $response[0] = 200;
            }
            if (!isset($response[1])) {
                $response[1] = [];
            }
            if (!isset($response[2])) {
                $response[2] = $return;
            }
            return $response;
        }
        return $return;
    }
}
