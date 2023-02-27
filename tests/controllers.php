<?php
use Pux\Controller\RESTfulController;
use Pux\Controller\ExpandableController;
use Pux\Controller\Controller;

class HelloController extends Controller
{
    public function index($name): string
    {
        return sprintf('Hello %s', $name);
    }

    public function show(): string
    {
        return "response";
    }
}

class HelloController2 extends Controller
{
    public function helloAction($name): string
    {
        return sprintf('hello %s', $name);
    }
}

class ProductController
{
    public function indexAction(): string { return 'index'; }

    public function fooAction(): string { return 'foo'; }

    public function barAction(): string { return 'bar'; }

    public function itemAction($id): string  { 
        return sprintf('product item %s', $id);
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
    public function createAction() { }

    /**
     * @Route("/:id")
     * @Method("POST")
     */
    public function updateAction($id) { }

    /**
     * @Route("/:id")
     * @Method("GET")
     */
    public function loadAction($id) { }

    /**
     * @Route("/:id");
     * @Method("DELETE")
     */
    public function deleteAction($id) { }

}


class PageController {

    public function page1(): string { return 'page1'; }

    public function page2(): string { return 'page2'; }

}

