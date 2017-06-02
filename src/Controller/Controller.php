<?php
namespace Pux\Controller;
use ReflectionClass;
use ReflectionObject;
use Universal\Http\HttpRequest;
use Pux\Mux;

class Controller
{
    /**
     * @var array PHPSGI compatible environment array (derived from $_SERVER)
     */
    protected $environment = array();


    /**
     * @var array Response array in [ code, headers, content ]
     */
    protected $response = array();

    /**
     * @var Universal\Http\HttpRequest object
     */
    protected $_request;


    /**
     * @var array The matched route array
     */
    protected $matchedRoute;

    /**
     * 
     * @param array $environment the default empty array was kept for backward compatibility.
     */
    public function __construct(array $environment = array(), array $response = array(), array $matchedRoute = null)
    {
        $this->environment  = $environment;
        $this->response     = $response;
        $this->matchedRoute = $matchedRoute;
    }


    /**
     * Create another controller with the current environment array
     *
     * NOTE: this is not supported in extension yet.
     *
     * @param string $controllerClass
     */
    protected function createController($controllerClass)
    {
        $cls = new ReflectionClass($controllerClass);
        return $cls->newInstance($this->environment);
    }

    public function init() { }

    public function prepare() {  }

    public function finalize() {  }


    public function getEnvironment()
    {
        return $this->environment;
    }

    public function getResponse()
    {
        return $this->response;
    }



    public function hasMatchedRoute()
    {
        return $this->matchedRoute ? true : false;
    }


    /**
     * @return array The standard route structure
     */
    public function getMatchedRoute()
    {
        return $this->matchedRoute;
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

    public function toJson($data, $encodeFlags = null)
    {
        if ($encodeFlags === null) {
            $encodeFlags = JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE;
        }
        return [200, ['Content-Type: application/json'], json_encode($data, $encodeFlags)];
    }


    /**
     * Run controller action
     *
     * @param string $action Action name, the action name should not include "Action" as its suffix.
     * @param array $vars    Action method parameters, which will be applied to the method parameters by their names.
     * @return string        Return execution result in string format.
     */
    public function runAction($action, array $vars = array())
    {
        $method = $action . 'Action';
        if ( ! method_exists($this,$method) ) {
            throw new Exception("Controller method $method does not exist.");
        }

        $this->prepare();

        $ro = new ReflectionObject( $this );
        $rm = $ro->getMethod($method);

        // Map vars to function arguments
        $parameters = $rm->getParameters();
        $arguments = array();
        foreach ($parameters as $param) {
            if ( isset( $vars[ $param->getName() ] ) ) {
                $arguments[] = $vars[ $param->getName() ];
            }
        }
        $ret = call_user_func_array( array($this,$method) , $arguments );

        // Trigger the after action
        $this->finalize();
        return $ret;
    }


    /**
     * Forward to another controller action
     *
     * @param string|Controller $controller A controller class name or a controller instance.
     * @param string            $actionName The action name
     * @param array             $parameters Parameters for the action method
     *
     *
     *  return $this->forward('\OAuthPlugin\Controller\AuthenticationErrorPage','index',array(
     *      'vars' => array(
     *          'message' => $e->lastResponse
     *      )
     *  ));
     */
    public function forward($controller, $actionName = 'index' , $parameters = array())
    {
        if (is_string($controller)) {
            $controller = new $controller;
        }
        return $controller->runAction($actionName, $parameters);
    }

    /**
     * Check if the controller action exists
     *
     * @param  string  $action action name
     * @return boolean
     */
    public function hasAction($action)
    {
        if (method_exists($this, $action . 'Action')) {
            return $action . 'Action';
        }
        return false;
    }

}
