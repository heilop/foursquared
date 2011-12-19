if (jQuery) {
  jQuery(document).ready(function($) {
    // Log the user's current location
    if (navigator.geolocation) {
      navigator.geolocation.getCurrentPosition(function(position) {
        var url = '/foursquared/location/' + position.coords.latitude + ',' + position.coords.longitude;
        $.getJSON(url, {}, function(json, textStatus) {
          console.log('Saved location for user');
        });
      });
    }
  });
}