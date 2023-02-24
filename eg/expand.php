<?php

require __DIR__ . '/../vendor/autoload.php';

use Pux\Controller\ExpandableController;
use Pux\RouteExecutor;
use Pux\Mux;
use Pux\Router;

class MyController extends Controller {
    public function indexAction(): string {
        return 'MyController::indexAction()!';
    }

    public function helloAction(): string {
        return 'MyController::helloAction()!';
    }

    /**
     * @uri /foo
     */
    public function overrideAction(): string {
        return 'MyController::overrideAction(), NOT MyController::fooAction()!';
    }
}


$controller = new MyController();

$mux = new Mux();
$mux->mount('/', $controller->expand());

$route = $mux->dispatch($_SERVER['REQUEST_URI']);
printf("Response: %s\n", RouteExecutor::execute($route));
