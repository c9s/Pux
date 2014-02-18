<?php

require '../vendor/autoload.php';

use Pux\Controller;
use Pux\Executor;
use Pux\Mux;
use Pux\Router;

class MyController extends Controller {
    public function indexAction() {
        return 'MyController::indexAction()!';
    }

    public function helloAction() {
        return 'MyController::helloAction()!';
    }

    /**
     * @uri /foo
     */
    public function overrideAction() {
        return 'MyController::overrideAction(), NOT MyController::fooAction()!';
    }
}


$controller = new MyController();

$mux = new Mux();
$mux->mount('/', $controller->expand());

$route = $mux->dispatch($_SERVER['REQUEST_URI']);
printf("Response: %s\n", Executor::execute($route));
