<?php
use Pux\PatternCompiler;

class PatternCompilerTest extends PHPUnit_Framework_TestCase
{

    public function testOptional()
    {
        $route = PatternCompiler::compile('/blog/:id(.:format)', array( 
            'default' => array(
                'format' => 'json'
            )
        ));

        ok( is_array($route) );

        ok( $route['variables'] );
        ok( in_array('format',$route['variables'] ) );
        ok( in_array('id',$route['variables'] ) );

        ok( preg_match( $route['compiled'] , '/blog/23.json', $matched ) );
        is( '23'   , $matched['id'] );
        is( 'json' , $matched['format'] );

        ok( preg_match( $route['compiled'] , '/blog/31.xml', $matched ) );
        is( '31'  , $matched['id'] );
        is( 'xml' , $matched['format'] );

        ok( preg_match( $route['compiled'] , '/blog/foo.yaml', $matched ) );
        is( 'foo'  , $matched['id'] );
        is( 'yaml' , $matched['format'] );


        ok( preg_match( $route['compiled'] , 
            '/blog/24.json', $matched ) );
        is( 'json', $matched['format'] );

        ok( ! preg_match( $route['compiled'] , 
            '/blog/23/json' ) );

        ok( ! preg_match( $route['compiled'] , 
            '/blog/23/' ) );
    }


    function testOptionalHolder()
    {
        $route = PatternCompiler::compile('/blog/:id.:format', array( 
            'default' => array(
                'format' => 'json'
            )
        ));
        ok( preg_match( $route['compiled'] , '/blog/23.json', $matched ) );
        is( 23, $matched['id'] );
        is( 'json' , $matched['format'] );

        ok( preg_match( $route['compiled'] , '/blog/23', $matched ) );
        is( 23, $matched['id'] );
    }

    function testPlaceHolder()
    {
        $route = PatternCompiler::compile('/blog/:year/:month');
        $pattern = '#^    /blog
    /(?P<year>[^/]+?)
    /(?P<month>[^/]+?)
$#xs';
        is($pattern,$route['compiled']);
    }
}

