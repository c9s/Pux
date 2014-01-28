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
    const TOKEN_TYPE_OPTIONAL = 1;
    const TOKEN_TYPE_VARIABLE = 2;
    const TOKEN_TYPE_TEXT = 3;

    /**
     * compile pattern
     *
     * @param string $pattern
     * @param array $options
     */
    static function compilePattern($pattern, $options = array() ) 
    {

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
            // match[0][1] // matched position for pattern.


            /*
             * Split tokens from abstract pattern
             * to rebuild regexp pattern.
             */
            if ($text = substr($pattern, $pos, $match[0][1] - $pos)) {
                $tokens[] = array( self::TOKEN_TYPE_TEXT, $text);
            }

            // the first char from pattern (which is the seperater)
            $seps = array($pattern[$pos]);
            $pos = $match[0][1] + strlen($match[0][0]);


            // generate optional pattern recursively
            if( $match[0][0][0] == '(' ) {
                $optional = $match[2][0];
                $subroute = self::compilePattern($optional,array(
                    'default'   => isset($options['default']) ? $options['default'] : null,
                    'require'   => isset($options['require']) ? $options['require'] : null,
                    'variables' => isset($options['variables']) ? $options['variables'] : null,
                ));

                $tokens[] = array( 
                    self::TOKEN_TYPE_OPTIONAL,
                    $optional[0],
                    $subroute['regex'],
                );
                foreach( $subroute['variables'] as $var ) {
                    $variables[] = $var;
                }
            } else {
                // generate a variable token 
                $varName = $match[1][0];

                // if we defined a pattern for this variable, we should use the given pattern..
                if ( isset( $options['require'][$varName] ) && $req = $options['require'][$varName]) {
                    $regexp = $req;
                } else {
                    if ($pos !== $len) {
                        $seps[] = $pattern[$pos];
                    }
                    // use the default pattern (which is based on the separater charactors we got)
                    $regexp = sprintf('[^%s]+?', preg_quote(implode('', array_unique($seps)), '#'));
                }

                // append token item
                $tokens[] = array(self::TOKEN_TYPE_VARIABLE, 
                    $match[0][0][0], 
                    $regexp, 
                    $varName);

                // append variable name
                $variables[] = $varName;
            }
        }

        if ($pos < $len) {
            $tokens[] = array(self::TOKEN_TYPE_TEXT, substr($pattern, $pos));
        }

        // find the first optional token
        $firstOptional = INF;
        for ($i = count($tokens) - 1; $i >= 0; $i--) {
            if ( self::TOKEN_TYPE_VARIABLE === $tokens[$i][0] 
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

        // indentation level
        $indent = 1;


        // token item structure:
        //   [0] => token type,
        //   [1] => separator
        //   [2] => pattern
        //   [3] => name, 

        // first optional token and only one token.
        if (1 === count($tokens) && 0 === $firstOptional) {
            $token = $tokens[0];
            ++$indent;

            // output regexp with separator and
            $regex .= str_repeat(' ', $indent * 4) . sprintf("%s(?:\n", preg_quote($token[1], '#'));

            // regular expression with place holder name. (?P<name>pattern)
            $regex .= str_repeat(' ', $indent * 4) . sprintf("(?P<%s>%s)\n", $token[3], $token[2]);

        } else {
            foreach ($tokens as $i => $token) {

                switch ( $token[0] ) {
                case self::TOKEN_TYPE_TEXT:
                    $regex .= str_repeat(' ', $indent * 4) . preg_quote($token[1], '#')."\n";
                    break;
                case self::TOKEN_TYPE_OPTIONAL:
                    // the question mark is for optional, the optional item may contains multiple tokens and patterns
                    $regex .= str_repeat(' ', $indent * 4) . "(?:\n" . $token[2] . str_repeat(' ', $indent * 4) . ")?\n";
                    break;
                default:
                    // append new pattern group for the optional pattern
                    if ($i >= $firstOptional) {
                        $regex .= str_repeat(' ', $indent * 4) . "(?:\n";
                        ++$indent;
                    }
                    $regex .= str_repeat(' ', $indent * 4). sprintf("%s(?P<%s>%s)\n", preg_quote($token[1], '#'), $token[3], $token[2]);
                    break;
                }
            }
        }

        // close groups
        while (--$indent) {
            $regex .= str_repeat(' ', $indent * 4).")?\n";
        }

        // save variables
        $options['variables'] = $variables;
        $options['regex'] = $regex;
        $options['tokens'] = $tokens;
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
        preg_match_all('/(?:
            # parse variable token with separator
            .            # separator
            :([\w\d_]+)  # variable
            |
            # optional tokens
            \((.*)\)
        )/x', $string, $matches, PREG_OFFSET_CAPTURE | PREG_SET_ORDER);
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


