Foursquared Drupal Module
=========================

In short: provides integration with Foursquare for Drupal 7.x.

Dependencies
------------

* __jQuery__ (it saves a lifetime of development work)
* __HTML5__ (_If_ you want to check in based on browser location)

Setup
-----

1. Clone or download the module into your modules folder of sites (or contrib, if you use Features (<3))
2. Register your application on Foursquare if you haven't already
3. Enter your client ID, Secret and registered URI callback on the settings page of this module.
4. Set permissions for your user to authorize foursquare

Current features
----------------

* Authentication/authorization with Foursquare
* Logging the user's current location
* Retrieving nearby venues based on current location

Wanted list of features
-----------------------

* AJAX authentication.
* Retrieval and theming of Check-ins.
* Logging the user's current location, _but_ falling back on IP-based location.
* Checking in.