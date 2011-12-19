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
    global $user;
    
    // Handle the given code
    $code = $_GET['code'];
    
    // Our necessary variables
    global $base_url;
    $client_id = variable_get('foursquared_client_id', FALSE);
    $redirect_uri = variable_get('foursquared_redirect_uri', $base_url . '/foursquared/authorize/');
    $secret = variable_get('foursquared_secret', '');
    
    // Build URL
    $fq_base_url = 'https://foursquare.com/oauth2/access_token?';
    $variables = array();
    $variables[] = "client_id=$client_id";
    $variables[] = "client_secret=$secret";
    $variables[] = "grant_type=authorization_code";
    $variables[] = "redirect_uri=$redirect_uri";
    $variables[] = "code=$code";
    $auth_url = $fq_base_url . implode('&', $variables);
    
    // Redirect to receive an access token
    // This may take a while.
    // TODO: make this into a task.
    $response = file_get_contents($auth_url);
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
 * Retrieves a list of nearby locations from Foursquare.
 *
 * @param string $token
 * @param string $location
 * @param string $search 
 * @param bool $json_output
 * @return mixed
 * @author Maarten Jacobs
 */
function foursquared_get_near_locations($token, $location, $search = NULL, $json_output = FALSE) {
  $venue_url = 'https://api.foursquare.com/v2/venues/search?';
  
  $variables = array();
  
  // Add the location of the current users
  $variables[] = 'll=' . $location;
    
  // Add the access token
  $variables[] = 'oauth_token=' . $token;
  
  // Add the search variable to the query
  if ($search) {
    $variables[] = 'query=' . $search;
  }
  
  // Create correct URL and retrieve our response
  $venue_url .= implode('&', $variables);
  $query_result = @file_get_contents($venue_url);
  
  // Check and double-check for content
  $venues = FALSE;
  if ($query_result) {
    $venue_response = json_decode($query_result);
    if (array_key_exists(0, $venue_response->response->groups)) {
      // And now we have venues.
      $venues = $venue_response->response->groups[0]->items;
    }
  }
  
  if ($json_output) {
    drupal_json_output($venues);
  } else {
    return $venues;
  }
}