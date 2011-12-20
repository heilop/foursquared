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
  
  // Build request
  $variables = array(
    'll=' . $location
  );
  // Add the search variable to the query
  if ($search) {
    $variables[] = 'query=' . $search;
  }
  
  // Send request
  $query_result = _foursquared_make_request(
    'https://api.foursquare.com/v2/venues/search?',
    $variables,
    $token
  );
  
  // Check and double-check for content
  $venues = FALSE;
  if ($query_result) {
    $venue_response = json_decode($query_result);
    if (is_array($venue_response->response->venues)) {
      // And now we have venues.
      $venues = $venue_response->response->venues;
    }
  }
  
  if ($json_output) {
    drupal_json_output($venues);
  } else {
    return $venues;
  }
}

/**
 * Given a valid access token, returns a user object that represents the user on Foursquare.
 *
 * @param string $token 
 * @return mixed
 * @author Maarten Jacobs
 */
function foursquared_get_self($token) {
  // Simple caching of self retrieval
  static $self;
  
  if (!is_object($self)) {
    // Build and send request
    $query_result = _foursquared_make_request(
      'https://api.foursquare.com/v2/users/self',
      array(),
      $token
    );

    // Return appropriate result
    if ($query_result) {
      // Return the inner user object
      $response = json_decode($query_result);
      $self = $response->response->user;
    } else {
      return FALSE;
    }
  } 
  
  return $self;
}

/**
 * Returns the Foursquare friends of a user.
 *
 * @param string $token
 * @param int $user_id 
 * @return mixed
 * @author Maarten Jacobs
 */
function foursquared_get_friends($token, $user_id) {
  
  // Build and send request
  $parsed_url = sprintf('https://api.foursquare.com/v2/users/%d/friends', $user_id);
  $request = _foursquared_make_request(
    $parsed_url,
    array(),
    $token
  );
  
  // Deal with response
  if (!$request) {
    return FALSE;
  } else {
    $decoded_response = json_decode($request);
    return $decoded_response->response->friends->items;
  }
  
}

/**
 * Wrapper function for get_friends with current Foursquare user.
 *
 * @param string $token 
 * @return mixed
 * @author Maarten Jacobs
 */
function foursquared_get_own_friends($token) {
  
  // Get current foursquare user
  $curr_user = foursquared_get_self($token);
  if (!$curr_user) {
    return FALSE;
  }
  
  $user_id = $curr_user->id;
  $friends = foursquared_get_friends($token, $user_id);
  
  if (!$friends) {
    return FALSE;
  } else {
    return $friends;
  }
  
}

/**
 * Makes a request to the Foursquare API.
 * The token can be false or evaluate to false and will not be used if so.
 * Returns FALSE on error and JSON (string) result on success.
 *
 * @param string $req_url 
 * @param array $variables 
 * @param string $token 
 * @return mixed
 * @author Maarten Jacobs
 */
function _foursquared_make_request($req_url, $variables, $token = FALSE) {
  // Type forcing
  if (!is_string($req_url) || strlen($req_url) === 0) {
    return FALSE;
  }
  if (!is_array($variables)) {
    $variables = array();
  }
  if ($token == FALSE && $token !== FALSE) {
    $token = FALSE;
  }
  if ($token !== FALSE && !is_string($token)) {
    return FALSE;
  }
  
  // Append '?' to the request url if it doesn't end in '?'
  $req_url_len = strlen($req_url);
  if ($req_url[$req_url_len - 1] !== '?') {
    $req_url .= '?';
  }
  
  // Add the token variable of the user, if any
  if ($token !== FALSE) {
    $variables[] = 'oauth_token=' . $token;
  }
  
  // Add the version variable, to prevent sudden deprecated functions
  $variables[] = 'v=20111220';
  
  // Build and use full request url
  $full_req_url = $req_url . implode('&', $variables);
  $query_result = @file_get_contents($full_req_url);
  
  if (!$query_result) {
    // We have an error :(
    return FALSE;
  } else {
    return $query_result;
  }
}