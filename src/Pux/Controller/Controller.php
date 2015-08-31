<?php
namespace Pux\Controller;
use ReflectionClass;
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

    public function init()
    {

    }


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

    public function toJson($data, $encodeFlags = JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE)
    {
        return json_encode($data, $encodeFlags);
    }
}
