<?php
use Pux\Controller\RESTfulController;
use Pux\Controller\ExpandableController;
use Pux\Controller\Controller;

class HelloController
{
    public function index($name) {
        return "Hello $name";
    }

    public function show() {
        return "response";
    }
}


class HelloController2
{
    public function helloAction($name) {
        return "hello $name";
    }
}

class ProductController
{
    public function indexAction() { return 'index'; }
    public function fooAction() { return 'foo'; }
    public function barAction() { return 'bar'; }

    public function itemAction($id)  { 
        return "product item $id";
    }
}


class ExpandableProductController extends ExpandableController
{




    public function indexAction() { }


    /**
     * @Method("GET")
     */
    public function itemAction() { }


    /**
     * @Route("/add")
     * @Method("POST")
     */
    public function addAction() { }

    /**
     * @Route("/update")
     * @Method("PUT")
     */
    public function updateAction() { }

    /**
     * @Route("/delete")
     * @Method("DELETE")
     */
    public function delAction() { }


    public function fooBarAction() { }
}


class ProductResourceController extends RESTfulController
{
    /**
     * @Method("POST")
     * @Route("");
     */
    public function createAction() {

    }

    /**
     * @Route("/:id")
     * @Method("POST")
     */
    public function updateAction() {

    }

    /**
     * @Route("/:id")
     * @Method("GET")
     */
    public function getAction() {

    }

    /**
     * @Route("/:id");
     * @Method("DELETE")
     */
    public function deleteAction() {

    }

}


class PageController {

    public function page1() { return 'page1'; }
    public function page2() { return 'page2'; }

}

