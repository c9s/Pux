CHANGELOG
==================

### Version 2.0

Newly added features:

- Added corneltek/universal package.
- Added Pux/RouteRequest class for dispatching routes with request constraints.
- Added Middleware design (see `Pux\Middleware\*` for more detials)
- Added GeocoderMiddleware
- Added HeadMiddleware
- Added ContentNegotiationMiddleware

- Added `Pux\Dispatchable` interface.

Incompatible changes:

- Renamed Executor => RouteExecutor
- Removed expand property
- Moved `Pux\Controller` to `Pux\Controller\Controller`.
- Moved mux expanding logics `expand()` from `Pux\Controller` to `Pux\Controller\ExpandableController`.

Fixes:

- Fixed pattern compiler for post-separators. e.g. `/:year-:month` or `/blog/:id(.:format)`
- Fixed action method annotation parser

Deprecation:

- Disabled `Mux->expand` property to enhance submux dispatching.



### Version 1.5.0

- The controller method getActionMethods() now returns an associative array with "methodName" => [ annotation, meta ]
- The controller method getActionRoutes()  now returns an indexed array for the [ "route path", {route options} ]
- Annotations from parent class methods are now parsed when calling `getActionMethods` or `getActionRoutes`
- When the child method has no annotation, we parse the parent method
  annotation for it. unless the child method has its own annotations.

### Version 1.4.0

- Add `pux_persistent_dispatch` function for persistent dispatching.
- Dispatch performance optimization, especially `__set_state` method call, 1.5x faster than before.

### Version 1.3.3

- Fix dispatch rtrim bug for pure PHP version.

### Version 1.3.2

- phar file re-compiled.

### Version 1.3.1

- dispatch performance improvement.
- `__set_state` construction time improvement.
- fix various bugs for zts (zend thread safe mode enabled)
