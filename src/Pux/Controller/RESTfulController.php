<?php
namespace Pux\Controller;
use Pux\Controller\Controller;
use Pux\Mux;

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
    public function expand($dynamic = false)
    {
        $mux   = new Mux();
        $target = $dynamic ? $this : $this->getClass();
        $mux->add('/:id', [$target, 'updateAction'], [ 'method' => REQUEST_METHOD_POST ]);
        $mux->add('/:id', [$target, 'getAction'], [ 'method' => REQUEST_METHOD_GET ]);
        $mux->add('/:id', [$target, 'deleteAction'], [ 'method' => REQUEST_METHOD_DELETE ]);
        $mux->add('', [$target, 'createAction'], [ 'method' => REQUEST_METHOD_POST ]);
        $mux->add('', [$target, 'getCollectionAction'], [ 'method' => REQUEST_METHOD_GET ]);
        return $mux;
    }


    public function code($code)
    {
        http_response_code($code);
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

