<?php
namespace Pux\Controller;
use ReflectionClass;
use ReflectionMethod;
use Universal\Http\HttpRequest;
use Pux\Mux;
use Pux\Expandable;

class ExpandableController extends Controller implements Expandable
{
    /**
     * @var Pux\Mux
     */
    protected $mux;


    /**
     * @param string $method request method
     * @return array Annotation info
     */
    protected function parseMethodAnnotation(ReflectionMethod $method)
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
     * getActionMethods parses the route definition from annotation and return
     * the "method" => "route meta" data structure
     *
     * Should always keep it returns simple array, so that we can cache the
     * data in somewhere...
     *
     * @return array
     */
    public function getActionMethods()
    {
        $refClass = new ReflectionClass($this);
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
                $annotations = $this->parseMethodAnnotation($method);

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
    protected function translatePath($methodName)
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
    public function getActionRoutes()
    {
        $routes = array();
        $actions = $this->getActionMethods();

        foreach ($actions as $actionName => $actionInfo) {
            list($annotations, $meta) = $actionInfo;

            if (isset($annotations['Route'])) {
                $path = $annotations['Route'];
            } else {
                if ($actionName === 'indexAction') {
                    $path = '';
                } else {
                    $path = $this->translatePath($actionName); // '/' . preg_replace_callback('/[A-Z]/', function($matches) {
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

    public function getMux()
    {
        return $this->mux;
    }


    /**
     * Expand controller actions to Mux object.
     *
     * @return Mux
     */
    public function expand(array $options = array(), $dynamic = false)
    {
        $this->mux = $mux = new Mux();
        $routes = $this->getActionRoutes();
        foreach ($routes as $route) {
            if ($dynamic) {
                $mux->add($route[0], array($this, $route[1]), array_merge($options, $route[2]));
            } else {
                $mux->add($route[0], array(get_class($this), $route[1]), array_merge($options, $route[2]));
            }
        }
        $mux->sort();
        return $mux;
    }
}
