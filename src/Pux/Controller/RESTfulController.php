<?php
namespace Pux\Controller;
use Pux\Controller;

abstract class RESTfulController extends Controller
{

    /**
     * @Method("POST")
     * @Route("");
     */
    public function createAction()
    {

    }

    /**
     * @Route("/:id")
     * @Method("POST")
     */
    public function updateAction()
    {

    }

    /**
     * @Route("/:id")
     * @Method("GET")
     */
    public function getAction()
    {

    }

    /**
     * @Route("/:id");
     * @Method("DELETE")
     */
    public function deleteAction()
    {

    }



    /**
     * Expand controller actions into Mux object
     *
     * @return Mux
     */
    public function expand()
    {
        $mux    = new Mux();
        $paths  = $this->getActionRoutes();
        foreach ($paths as $path) {
            $mux->add($path[0], array(get_class($this), $path[1]), $path[2]);
        }
        $mux->sort();
        return $mux;
    }


    /**
     * HTTP Status Code:
     *
     * @link http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
     * @link http://restpatterns.org/HTTP_Status_Codes
     *
     * REST Pattern
     * @link http://restpatterns.org/
     */
    public function codeOk() 
    {
        header("HTTP/1.1 200 OK");
    }

    public function codeCreated()
    {
        header('HTTP/1.1 201 Created');
    }

    public function codeAccepted()
    {
        header('HTTP/1.1 202 Accepted');
    }

    public function codeNoContent() 
    {
        header('HTTP/1.1 204 No Content');
    }

    public function codeBadRequest()
    {
        header('HTTP/1.1 400 Bad Request');
    }

    public function codeForbidden()
    {
        header('HTTP/1.1 403 Forbidden');
    }

    public function codeNotFound()
    {
        header('HTTP/1.1 404 Not Found');
    }

    public function getClass()
    {
        return get_class($this);
    }
}

