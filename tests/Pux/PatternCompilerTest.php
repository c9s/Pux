<?php
use Pux\PatternCompiler;

class PatternCompilerTest extends PHPUnit_Framework_TestCase
{

    public function testCompiledPatternWithPostSeparator()
    {
        $route = PatternCompiler::compile('/blog/:year-:month', array());
        $this->assertNotEmpty($route);
        $this->assertTrue(is_array($route));
        $this->assertEquals(1, preg_match($route['compiled'] , '/blog/2013-09', $matches));
        $this->assertEquals('2013', $matches['year']);
        $this->assertEquals('09', $matches['month']);
    }

    public function testRESTfulPatternWithoutFormat()
    {
        $route = PatternCompiler::compile('/blog/:id(.:format)', array());
        $this->assertNotEmpty($route);
        $this->assertTrue(is_array($route));

        $this->assertEquals(1, preg_match($route['compiled'] , '/blog/3', $matches));
        $this->assertEquals('3', $matches['id']);
        $this->assertFalse(isset($matches['json']));
    }

    public function testRESTfulPatternWithOptionalFormat()
    {
        $route = PatternCompiler::compile('/blog/:id(.:format)', array());
        $this->assertNotEmpty($route);
        $this->assertTrue(is_array($route));
        $this->assertEquals(1, preg_match($route['compiled'] , '/blog/3314.json', $matches));
        $this->assertEquals('3314', $matches['id']);
        $this->assertEquals('json', $matches['format']);
    }


    public function testOptional()
    {
        $route = PatternCompiler::compile('/blog/:id(.:format)', array( 
            'default' => array(
                'format' => 'json'
            )
        ));

        $this->assertTrue( is_array($route) );

        ok( $route['variables'] );
        $this->assertTrue( in_array('format',$route['variables'] ) );
        $this->assertTrue( in_array('id',$route['variables'] ) );

        ok( preg_match( $route['compiled'] , '/blog/23.json', $matched ) );
        $this->assertEquals('23'   , $matched['id']);
        $this->assertEquals('json' , $matched['format'] );

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
    /(?P<year>[^/]+)
    /(?P<month>[^/]+)
$#xs';
        is($pattern,$route['compiled']);
    }
}

