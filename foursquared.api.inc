<?php
/**
 * Handles the authorization of the app for the current user.
 *
 * @return void
 * @author Maarten Jacobs
 */
function foursquared_handle_authorization() {
  // The current URL (in $_GET) should provide a code, as to whether we were successfully authorized.
  if (!isset($_GET['code'])) {
    drupal_set_message('Did not receive a code from Foursquare.', 'error');
  } else {
    // Handle the given code
    $code = $_GET['code'];
    
    // Save authorised state for user
    global $user;
    db_query('UPDATE {users} SET foursquared_authorized = 1 WHERE uid = :uid', array('uid' => $user->uid));
    
    // Our necessary variables
    global $base_url;
    $client_id = variable_get('foursquared_client_id', FALSE);
    $redirect_uri = variable_get('foursquared_redirect_uri', $base_url . '/foursquared/authorize/');
    $secret = variable_get('foursquared_secret', '');
    
    // Redirect to receive an access token
    // This may take a while.
    // TODO: make this into a task.
    $response = file_get_contents("https://foursquare.com/oauth2/access_token?client_id=$client_id&client_secret=$secret&grant_type=authorization_code&redirect_uri=$redirect_uri&code=$code"
    );
    if ($response) {
      $resp_inner = json_decode($response, true); // Screw objects.
      $access_token = $resp_inner['access_token'];
      if ($access_token) {
        db_query(
          'UPDATE {users} SET foursquared_access_token = :token WHERE uid = :uid', 
          array('token' => $access_token, 'uid' => $user->uid)
        );
      } 
    } 
    
    // Handle errors and success
    if (!$response || !isset($access_token)) {
      drupal_set_message('No response received from Foursquare from authentication. Please confirm authorization and try again.', 'error');
    } else {
      // It worked. Hurrah!
      drupal_set_message('Successfully authenticated Foursquare.');
    }  
  }  
  drupal_goto(''); // Go back to the homepage
}

/**
 * Retrieves a list of nearby locations from Foursquare
 *
 * @param string $search 
 * @return void
 * @author Maarten Jacobs
 */
function foursquared_get_near_locations($token, $location, $search = NULL, $json_output = FALSE) {
  $venue_url = 'https://api.foursquare.com/v2/venues/search?';
  
  $variables = array();
  
  // Add the location of the current users
  $variables[] = 'll=' . $location;
    
  // Add the access token
  $variables[] = 'oauth_token=' . $token;
  
  // Create correct URL and retrieve our response
  $venue_url .= implode('&', $variables);
  $query_result = @file_get_contents($venue_url);
  
  // Check and double-check for content
  $venues = FALSE;
  if ($query_result) {
    $venue_json = json_decode($query_result);
    if (array_key_exists(0, $venue_json->response->groups)) {
      // And now we have venues.
      $venues = $venue_json->response->groups[0]->items;
    }
  }
  
  return $venues;
}