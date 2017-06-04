<?php

namespace Pux\Builder;

use ReflectionClass;
use ReflectionMethod;
use Pux\Mux;

class ControllerRouteBuilder
{
    /**
     * @param string $method request method
     * @return array Annotation info
     */
    protected static function parseMethodAnnotation(ReflectionMethod $method)
    {
        $annotations = array();
        if ($doc = $method->getDocComment()) {
            if (preg_match('/^[\s*]*\@Method\("(get|put|post|delete|head|patch|options)"\)/im', $doc, $regs)) {
                $annotations['Method'] = $regs[1];
            }
            if (preg_match('/^[\s*]*\@Route\("([^\s]*)"\)/im', $doc, $regs)) {
                $annotations['Route'] = $regs[1];
            }
        }
        return $annotations;
    }

    /**
     * parseActionMethods parses the route definition from annotation and return
     * the "method" => "route meta" data structure
     *
     * Should always keep it returns simple array, so that we can cache the
     * data in somewhere...
     *
     * @return array
     */
    public static function parseActionMethods($con)
    {
        $refClass = new ReflectionClass($con);
        $methodMap = [];

        // build up parent class list
        $parentClasses = [];
        $parentClasses[] = $parentClassRef = $refClass;
        while ($parent = $parentClassRef->getParentClass()) {
            $parentClasses[] = $parent;
            $parentClassRef = $parent;
        }

        // iterate methods from parent class actions
        foreach (array_reverse($parentClasses) as $class) {
            foreach ($class->getMethods(ReflectionMethod::IS_PUBLIC) as $method) {

                // Ignore class methods that doesn't have Action in suffix
                if (!preg_match('/Action$/', $method->getName())) {
                    continue;
                }
                if (in_array($method->getName(), [ 'runAction', 'hasAction' ])) {
                    continue;
                }

                $meta = array('class' => $class->getName());
                $annotations = self::parseMethodAnnotation($method);

                // If it's empty, then fetch annotations from parent methods
                if (empty($annotations)) {
                    if (isset($methodMap[$method->getName()])) {
                        $annotations = $methodMap[$method->getName()][0];
                    }
                }
                // always update method Map
                $methodMap[$method->getName()] = array($annotations, $meta);
            }
        }
        // TODO: see if we can cache it.
        return $methodMap;
    }

    /**
     * Translate action method name into route path.
     *
     * Upper case letters will be translated into slash + lower letter, e.g.
     *
     *      pageUpdateAction => /page/update
     *      fooAction => /foo
     *
     * @return string path
     */
    protected static function translatePath($methodName)
    {
        $methodName = preg_replace('/Action$/', '', $methodName);
        return '/'.preg_replace_callback('/[A-Z]/', function ($matches) {
            return '/'.strtolower($matches[0]);
        }, $methodName);
    }

    /**
     * Return [["/path", "testAction", [ "method" => ... ] ],...].
     *
     * @return array returns routes array
     */
    public static function build($controller)
    {
        $routes = array();
        $actions = self::parseActionMethods($controller);

        foreach ($actions as $actionName => $actionInfo) {
            list($annotations, $meta) = $actionInfo;

            if (isset($annotations['Route'])) {
                $path = $annotations['Route'];
            } else {
                if ($actionName === 'indexAction') {
                    $path = '';
                } else {
                    $path = self::translatePath($actionName); // '/' . preg_replace_callback('/[A-Z]/', function($matches) {
                }
            }

            $route = array($path, $actionName);

            if (isset($annotations['Method'])) {
                $route[] = array('method' => Mux::convertRequestMethodConstant($annotations['Method']));
            } else {
                $route[] = array();
            }
            $routes[] = $route;
        }

        return $routes;
    }
}
