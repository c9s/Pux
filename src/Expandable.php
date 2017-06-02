<?php
namespace Pux;

interface Expandable
{

    /**
     * @param boolean $dynamic whether to add the current object to the mux,
     *                when $dynamic is false, expand method only adds the class
     *                name into mux object.
     */
    public function expand(array $options = array(), $dynamic = true);

}
