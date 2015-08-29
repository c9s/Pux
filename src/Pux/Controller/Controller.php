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

    public function toJson($data)
    {
        return json_encode($data);
    }
}
