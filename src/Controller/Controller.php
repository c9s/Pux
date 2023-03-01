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
    protected $environment = [];


    /**
     * @var array Response array in [ code, headers, content ]
     */
    protected $response = [];

    /**
     * @var Universal\Http\HttpRequest object
     */
    protected $_request;


    /**
     * @var array The matched route array
     */
    protected $matchedRoute = [];

    public function call(array & $environment, array $response)
    {
        $this->context($environment, $response);

        $action = $environment['pux.controller_action'];
        [$pcre, $pattern, $callbackArg, $options] = $this->matchedRoute;

        $reflectionClass = new ReflectionClass($this);

        $rps = $reflectionClass->getMethod($action)->getParameters();
        $vars = $options['vars'] ?? []
                ;

        $arguments = [];
        foreach ($rps as $rp) {
            $n = $rp->getName();
            if (isset($vars[ $n ])) {
                $arguments[] = $vars[ $n ];
            } elseif (isset($route[3]['default'][ $n ])
                            && $default = $route[3]['default'][ $n ]) {
                $arguments[] = $default;
            } elseif (!$rp->isOptional() && !$rp->allowsNull()) {
                throw new Exception('parameter is not defined.');
            }
        }

        return call_user_func_array([$this, $action], $arguments);
    }

    protected function context(array & $environment, array $response)
    {
        $this->environment  = $environment;
        $this->response     = $response;
        $this->matchedRoute = $environment['pux.route'];
    }

    public function hasMatchedRoute()
    {
        return (bool) $this->matchedRoute;
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
    protected function getRequest($recreate = false)
    {
        if (!$recreate && $this->_request) {
            return $this->_request;
        }

        return $this->_request = HttpRequest::createFromGlobals($this->environment);
    }

    protected function toJson($data, $encodeFlags = null)
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
    public function runAction($action, array $vars = [])
    {
        $method = sprintf('%sAction', $action);
        if (! method_exists($this,$method)) {
            throw new Exception(sprintf('Controller method %s does not exist.', $method));
        }

        $reflectionObject = new ReflectionObject( $this );
        $reflectionMethod = $reflectionObject->getMethod($method);

        // Map vars to function arguments
        $parameters = $reflectionMethod->getParameters();
        $arguments = [];
        foreach ($parameters as $parameter) {
            if (isset($vars[$parameter->getName()])) {
                $arguments[] = $vars[ $parameter->getName() ];
            }
        }

        return call_user_func_array( [$this, $method] , $arguments );
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
    public function forward($controller, $actionName = 'index' , $parameters = [])
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
        $method = sprintf('%sAction', $action);
        if (method_exists($this, $method)) {
            return $method;
        }

        return false;
    }
}
