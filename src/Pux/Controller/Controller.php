<?php
namespace Pux\Controller;
use ReflectionClass;
use Universal\Http\HttpRequest;
use Pux\Mux;

class Controller
{
    protected $environment = array();

    protected $_request;

    /**
     * 
     * @param array $environment the default empty array was kept for backward compatibility.
     */
    public function __construct(array $environment = array())
    {
        $this->environment = $environment;
    }

    /**
     * Create and Return HttpRequest object from the environment
     *
     * @param boolean $recreate
     * @return Universal\Http\HttpRequest
     */
    public function getRequest($recreate = false)
    {
        if (!$recreate && $this->_request) {
            return $this->_request;
        }
        return $this->_request = HttpRequest::createFromGlobals($this->environment);
    }


    /**
     * @param strign $method request method
     *
     * @return array Annotation info
     */
    protected function parseMethodAnnotation($method)
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

    protected function parseMethods(ReflectionClass $refObject, array &$args, $parent = 0)
    {
        if ($pClassRef = $refObject->getParentClass()) {
            $this->parseMethods($pClassRef, $args, 1);
        }

        $methods = $refObject->getMethods();
        foreach ($methods as $method) {
            if (!preg_match('/Action$/', $method->getName())) {
                return;
            }

            $meta = array('class' => $refObject->getName());
            $anns = $this->parseMethodAnnotation($method);
            if (empty($anns)) {
                // get parent method annotations
                if (isset($args[ $method->getName() ])) {
                    $anns = $args[$method->getName()][0];
                }
            }
            // override
            $args[ $method->getName() ] = array($anns, $meta);
        }
    }

    public function getActionMethods()
    {
        $refObject = new ReflectionClass($this);
        $args = array();
        $this->parseMethods($refObject, $args, 0);

        return $args;
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
                $route[] = array('method' => Mux::getRequestMethodConstant($annotations['Method']));
            } else {
                $route[] = array();
            }
            $routes[] = $route;
        }

        return $routes;
    }

    /**
     * Expand controller actions to Mux object.
     *
     * @return Mux
     */
    public function expand()
    {
        $mux = new Mux();
        $paths = $this->getActionRoutes();
        foreach ($paths as $path) {
            $mux->add($path[0], array(get_class($this), $path[1]), $path[2]);
        }
        $mux->sort();

        return $mux;
    }

    public function toJson($data)
    {
        return json_encode($data);
    }
}
