<?php
require 'SimpleBench/Task.php';
require 'SimpleBench/Utils.php';
require 'SimpleBench/ComparisonMatrix.php';
require 'SimpleBench/SystemInfo/Darwin.php';
require 'SimpleBench/MatrixWriter/Writer.php';
require 'SimpleBench/MatrixWriter/JsonWriter.php';
require 'SimpleBench/MatrixPrinter/EzcGraph.php';
require 'SimpleBench/MatrixPrinter/Console.php';
require 'SimpleBench.php';

// requirement from symfon
require '../src/Pux/PatternCompiler.php';
use Pux\Mux;

$mux = Pux\Mux::__set_state(array(
  'id' => NULL,
  'routes' => 
 array (
   0 => 
   array (
     0 => false,
     1 => '/hello',
     2 => 
     array (
       0 => 'HelloController',
       1 => 'helloAction',
     ),
     3 => 
     array (
     ),
   ),
 ),
  'routesById' => 
 array (
 ),
  'staticRoutes' => 
 array (
 ),
  'submux' => 
 array (
 ),
  'expand' => true,
)); /* version */


$bench = new SimpleBench;
$bench->setN( 10000 );

$bench->iterate( 'match' , function() use ($mux) {
    $route = $mux->match('/hello');
});

$bench->iterate( 'dispatch' , function() use ($mux) {
    $route = $mux->dispatch('/hello');
});

$bench->iterate( '__set_state' , function() {
    $mux = Pux\Mux::__set_state(array(
        'id' => NULL,
        'routes' => 
        array (
        0 => 
        array (
            0 => false,
            1 => '/hello',
            2 => 
            array (
            0 => 'HelloController',
            1 => 'helloAction',
            ),
            3 => 
            array (),),
        ),
        'routesById' => array (),
        'staticRoutes' => array (),
        'submux' => array (),
        'expand' => true,
    )); /* version */
});

$result = $bench->compare();
echo $result->output('console');
