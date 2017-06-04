<?php
namespace Pux\Controller;
use ReflectionClass;
use ReflectionObject;
use Universal\Http\HttpRequest;
use Pux\Mux;

use PHPSGI\App;

class Controller implements App
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
     */
    public function __construct()
    {

    }

    public function call(array & $environment, array $response)
    {
        // setup state
        $this->environment  = $environment;
        $this->response     = $response;


        $this->matchedRoute = $environment['pux.route'];

        $action = $environment['pux.controller_action'];

        list($pcre, $pattern, $callbackArg, $options) = $this->matchedRoute;

        $rc = new ReflectionClass($this);

        $rps = $rc->getMethod($action)->getParameters();
        $vars = isset($options['vars'])
                ? $options['vars']
                : array()
                ;

        $arguments = [];
        foreach ($rps as $rp) {
            $n = $rp->getName();
            if (isset($vars[ $n ])) {
                $arguments[] = $vars[ $n ];
            } else if (isset($route[3]['default'][ $n ])
                            && $default = $route[3]['default'][ $n ]) {
                $arguments[] = $default;
            } else if (!$rp->isOptional() && !$rp->allowsNull()) {
                throw new Exception('parameter is not defined.');
            }
        }

        return call_user_func_array([$this, $action], $arguments);
    }

    public function init() { }

    public function prepare() {  }

    public function finalize() {  }

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
        $method = "{$action}Action";
        if (! method_exists($this,$method)) {
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
        $method = "{$action}Action";
        if (method_exists($this, $method)) {
            return $method;
        }
        return false;
    }
}
