<?php
namespace Pux;
use Exception;

/**
 * Compile path string into PCRE pattern:
 *
 *   /blog/:year/:month
 *   /blog/item/:id
 *   /blog/item/:id(.:format)
 *
 */
class PatternCompiler
{


    /**
     * compile pattern
     *
     * @param string $pattern
     * @param array $options
     */
    static function compilePattern($pattern, $options = array() ) {

        $len = strlen($pattern);
        /**
         * contains:
         *   
         *   array( 'text', $text ),
         *   array( 'variable', $match[0][0][0], $regexp, $var);
         *
         */
        $tokens = array();
        $variables = array();
        $pos = 0;

        /**
         *  the path like:
         *
         *      /blog/to/:year/:month
         *
         *  will be separated like:
         *      
         *      [
         *          '/blog/to',  (text token)
         *          '/:year',    (reg exp token)
         *          '/:month',   (reg exp token)
         *      ]
         */
        $matches = self::splitTokens( $pattern );

        // build tokens
        foreach ($matches as $match) {
            
            /*
             * Split tokens from abstract pattern
             * to rebuild regexp pattern.
             */
            if ($text = substr($pattern, $pos, $match[0][1] - $pos)) {
                $tokens[] = array('text', $text);
            }

            // the first char from pattern (seperater)
            $seps = array($pattern[$pos]);
            $pos = $match[0][1] + strlen($match[0][0]);


            // optional pattern
            if( $match[0][0][0] == '(' ) {
                $optional = $match[2][0];
                $subroute = self::compilePattern($optional,array(
                    'default' => @$options['default'],
                    'require' => @$options['require'],
                    'variables' => @$options['variables'],
                ));


                $tokens[] = array( 
                    'optional',
                    $optional[0],
                    $subroute['regex'],
                );
                // $regexp = 
                foreach( $subroute['variables'] as $var ) {
                    $variables[] = $var;
                }
            }
            else {
                // field name (variable name)
                $var = $match[1][0];

                /* build field pattern from require */
                if ( isset( $options['require'][$var] ) && $req = $options['require'][$var]) {
                    $regexp = $req;
                } else {
                    if ($pos !== $len) {
                        $seps[] = $pattern[$pos];
                    }

                    // build regexp (from separater)
                    $regexp = sprintf('[^%s]+?', preg_quote(implode('', array_unique($seps)), '#'));
                }

                $tokens[] = array('variable', 
                    $match[0][0][0], 
                    $regexp, 
                    $var);
                $variables[] = $var;
            }
        }

        if ($pos < $len) {
            $tokens[] = array('text', substr($pattern, $pos));
        }

        // find the first optional token
        $firstOptional = INF;
        for ($i = count($tokens) - 1; $i >= 0; $i--) {
            if ('variable' === $tokens[$i][0] 
                && isset($options['default'][ $tokens[$i][3] ]) )
            {
                $firstOptional = $i;
            } 
            else 
            {
                break;
            }
        }

        // compute the matching regexp
        $regex = '';
        $indent = 1;


        // token structure:
        //   [0] => token type,
        //   [1] => separator
        //   [2] => pattern
        //   [3] => name , 

        // first optional token and only one token.
        if (1 === count($tokens) && 0 === $firstOptional) {
            $token = $tokens[0];
            ++$indent;
            $regex .= str_repeat(' ', $indent * 4)
                . sprintf("%s(?:\n", 
                    preg_quote($token[1], '#'));

            // regular expression with place holder name. ( 
            $regex .= str_repeat(' ', $indent * 4)
                . sprintf("(?P<%s>%s)\n", 
                    $token[3], $token[2]);

        } else {
            foreach ($tokens as $i => $token) {
                if ('text' === $token[0]) {
                    $regex .= str_repeat(' ', $indent * 4)
                            . preg_quote($token[1], '#')."\n";
                }
                elseif( 'optional' === $token[0]) {
                    $regex .= str_repeat(' ', $indent * 4) . "(?:\n";
                    $regex .= $token[2];
                    $regex .= str_repeat(' ', $indent * 4) . ")?\n";
                }
                else {
                    if ($i >= $firstOptional) {
                        $regex .= str_repeat(' ', $indent * 4)
                            . "(?:\n";
                        ++$indent;
                    }
                    $regex .= str_repeat(' ', $indent * 4).
                        sprintf("%s(?P<%s>%s)\n", 
                        preg_quote($token[1], '#'), $token[3], $token[2]);
                }
            }
        }
        while (--$indent) {
            $regex .= str_repeat(' ', $indent * 4).")?\n";
        }

        // save variables
        // $options['variables'] = $variables;
        $options['regex'] = $regex;
        // $options['tokens'] = $tokens;
        return $options;
    }


    /**
     * Split tokens from path.
     *
     * @param string $string path string
     *
     * @return array matched results
     */
    static function splitTokens($string)
    {
        // split with ":variable" and path
        preg_match_all('#(?:
            .:([\w\d_]+)
            |
            \((.*)\)
        )#x', $string, $matches, PREG_OFFSET_CAPTURE | PREG_SET_ORDER);
        return $matches;
    }

    /**
     * Compiles the current route instance.
     *
     * @param array $route route info
     *
     * @return array compiled route info, with newly added 'compiled' key.
     */
    static function compile($pattern, $options = array())
    {
        $route = self::compilePattern($pattern, $options);

        // save compiled pattern
        $route['compiled'] = sprintf("#^%s$#xs", $route['regex']);
        $route['pattern'] = $pattern; // save pattern
        return $route;
    }
}


