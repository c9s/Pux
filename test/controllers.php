<?php


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

class ExpandableProductController extends Pux\Controller
{
    public function indexAction() { }

    public function itemAction() { }


    /**
     * @method(POST)
     */
    public function addAction() { }

    /**
     * @method(PUT)
     */
    public function updateAction() { }

    /**
     * @method(DELETE)
     */
    public function delAction() { }

    public function fooBarAction() { }
}



class PageController {

    public function page1() { return 'page1'; }
    public function page2() { return 'page2'; }

}

