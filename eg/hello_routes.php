<?php
// require '../vendor/autoload.php';
use Phux\Mux;
$mux = new Mux;
$mux->add('/hello', ['HelloController','helloAction']);
return $mux;
