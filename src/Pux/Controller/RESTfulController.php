<?php
namespace Pux\Controller;
use Pux\Controller\ExpandableController;
use Pux\Mux;

abstract class RESTfulController extends ExpandableController
{
    /**
     * @Route("");
     * @Method("GET")
     */
    public function collectionAction()
    {

    }

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
    public function updateAction($id)
    {

    }

    /**
     * @Route("/:id")
     * @Method("GET")
     */
    public function loadAction($id)
    {

    }

    /**
     * @Route("/:id");
     * @Method("DELETE")
     */
    public function deleteAction($id)
    {

    }




    /**
     * Expand controller actions into Mux object
     *
     * @return Mux
     */
    public function expand(array $options = array(), $dynamic = false)
    {
        $mux   = new Mux();
        $target = $dynamic ? $this : $this->getClass();
        $mux->add('/:id', [$target, 'updateAction'], array_merge($options, array('method' => REQUEST_METHOD_POST   )));
        $mux->add('/:id', [$target, 'loadAction'],   array_merge($options, array('method' => REQUEST_METHOD_GET    )));
        $mux->add('/:id', [$target, 'deleteAction'], array_merge($options, array('method' => REQUEST_METHOD_DELETE )));
        $mux->add('', [$target, 'createAction'],     array_merge($options, array('method' => REQUEST_METHOD_POST   )));
        $mux->add('', [$target, 'collectionAction'], array_merge($options, array('method' => REQUEST_METHOD_GET    )));
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

