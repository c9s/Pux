<?php
// require '../vendor/autoload.php';
use Pux\Mux;
$mux = new Mux;
$mux->add('/hello', ['HelloController','helloAction']);
return $mux;
