<?php 
/**
 * @file
 */

// $Id$

require_once drupal_get_path('module', 'renren') . '/RESTClient.class.php';

class RenrenClient extends RESTClient {
  private $config = array('client_id' => '', 'client_secret' => '', 'redirect_uri' => '');
  private $response_type = 'code';
  private $token = '';
  // http://wiki.dev.renren.com/wiki/%E6%9D%83%E9%99%90%E5%88%97%E8%A1%A8 .
  private $scope = array(
    // Fetch information permissions
    'read_user_blog',
    'read_user_checkin',
    'read_user_feed',
    'read_user_guestbook',
    'read_user_invitation',
    'read_user_like_history',
    // Publish permissions
    'publish_blog',
    'publish_checkin',
    'publish_feed',
    'publish_share',
  );
  
  private $author_url = 'https://graph.renren.com/oauth/authorize';
  private $token_url = 'https://graph.renren.com/oauth/token';
  private $session_url = 'https://graph.renren.com/renren_api/session_key';
  private $server_url = 'http://api.xiaonei.com/restserver.do';
  
  public function init($config) {
    $this->config = array_merge($this->config, $config);
    unset($this->config['scope']);
    if (isset($config['scope'])) {
      $this->scope = array_merge($this->scope, $config['scope']);
    }

    if (empty($this->config['client_id']) || empty($this->config['client_secret']) || empty($this->config['redirect_uri'])) {
      throw new Exception(t('Invalid API URL or Client ID or Client Secret. Please make sure have right Renren API settings'));
    }
  }
  
  public function setScope($scope) {
    $this->scope += $scope;
  }

  
  public function generateAuthURL() {
    $params = 'response_type=code&display=page&scope=' . implode(' ', $this->scope) . '&redirect_uri=' . $this->config['redirect_uri'] . '&client_id=' . $this->config['client_id'];
    return $this->author_url . '?' . $params;
  }
  
  public function getAccessToken($auth_code) {
    if (!isset($auth_code)) throw new Exception('Invalid Authorization Code');
    $params = array(
      'grant_type' => 'authorization_code',
      'code' => $auth_code,
      'client_id' => $this->config['client_id'],
      'client_secret' => $this->config['client_secret'],
      'redirect_uri' => $this->config['redirect_uri'],
    );
    $response = $this->_POST($this->token_url, $params);
    if ($response->error) {
      throw new Exception('Error in the request access token');
      return NULL;
    }
    
    $_SESSION['renren_access_token'] = $response;
    
    return $response;
  }
  
  public function __call($method, $arguments) {
    if (!isset($_SESSION['renren_access_token'])) {
      return NULL;
    }
    $method = str_replace('_', '.', $method);
    dpm($method);
    $params = array(
      'access_token' => $_SESSION['renren_access_token']->access_token,
      'method' => $method,
      'call_id' => time(),
      'v' => '1.0',
      'format' => 'json',
    );
    if (is_array($arguments)) {
      $params = array_merge($params, $arguments[0]);
    }
    ksort($params);
    reset($params);
    
    $sig = '';
    foreach ($params as $key => $param) {
      $sig .= $key . '=' . $param;
    }

    $sig = md5($sig . $this->config['client_secret']);
    $params['sig'] = $sig;
    $response = $this->_POST($this->server_url, $params);
    
    return $response;
  }
}

/**
 * Init the Renren client Object
 */
function renren_init_client($client_id, $client_secret, $scope = array()) {
  $redirect_url = url('renren/redirect', array('absolute' => TRUE));
  if (empty($client_id) || empty($client_secret)) return;
  global $renren;
  try {
    $renren = new RenrenClient();
    $renren->init(array('client_id' => $client_id, 'client_secret' => $client_secret, 'redirect_uri' => $redirect_url));
  }
  catch (Exception $e) {
    drupal_set_message(t('Error in init the Renren connect client'));
  }
  $renren->setScope($scope);
}

/**
 * Generate the URL of auth
 */
function renren_get_authurl() {
  $renren = renren_client();
  if ($renren) {
    return $renren->generateAuthURL(); 
  }
  else {
    return NULL;
  }
}

/**
 * Use the code that returned by renren to get new token
 * @param $auth_code
 *  The return code by renren 
 */
function renren_fetch_token($auth_code = '') {
  $renren = renren_client();
  try {
    if (isset($_SESSION['renren_access_token'])) {
      return $_SESSION['renren_access_token'];
    }
    $response = $renren->getAccessToken($auth_code);
    return $response;
  }
  catch (Exception $e) {
    drupal_set_message(t('Error in get access token'));
  }
}

/**
 * Get the renren account profile
 */
function renren_user_profile() {
  $renren = renren_client();
  if (isset($_SESSION['renren_account'])) {
    return $_SESSION['renren_account'];
  }
  else {
    $renren_user = $renren->users_getInfo();
    if (!isset($renren_user->error_code)) {
      $_SESSION['renren_account'] = $renren_user[0];
    }
    return $renren_user[0]; 
  }
}

/**
 * 
 * Post seed to Renren
 * @param $description
 * @param $name
 * @param $url
 * @param $image
 * @param $caption
 * @param $action_name
 * @param $action_link
 * @param $message
 */
function renren_publish_seed($description, $name = '', $url = '', $image = '', $caption = '', $action_name = '', $action_link = '', $message = '') {
  if (empty($url)) {
    $url = url(NULL, array('absolute' => TRUE));
  }
  if (empty($name)) {
    $name = variable_get('site_slogan', '');
  }
  
  if (!empty($description)) {
    $renren = renren_client();
    $response = $renren->feed_publishFeed(array('name' => $name, 'description' => $description, 'url' => $url, 'caption' => $caption, 'image' => $image, 'action_name' => $action_name, 'action_link' => $action_link, 'message' => $message));
    if ($response->post_id) {
      drupal_set_message(t('Share success with Renren.'));
    }
    return $response;
  }
}

/**
 * Post blog on renren
 */
function renren_publish_blog($title, $content, $visible = '99', $password = '') {
  $renren = renren_client();
  $response = $renren->blog_addBlog(array('title' => $title, 'content' => $content, 'visible' => $visible, 'password' => $password));
  if ($response->id) {
    drupal_set_message(t('Success post blog on renren'));
  }
  return $response;
}

/**
 * Renren create account.
 */
function renren_create_account($renren_profile = NULL) {
  if ($renren_profile == NULL) {
    return;
  }
  $mapping_user = db_fetch_object(db_query('SELECT * FROM {renren_account_mapping} WHERE renren_uid = %d', array($renren_profile->uid)));
  if (!empty($mapping_user)) {
    return user_load($mapping_user->uid);
  }
  if (user_load(array('name' => check_plain($renren_profile->name)))) {
    // The user name already exit
    $name = $renren_profile->name .$renren_profile->uid;
  }
  else {
    $name = $renren_profile->name;
  }
  $user_edit = array(
      'name' => check_plain($name),
      'mail' => 'example@example.com',
      'pass' => user_password(),
      'status' => 1,
      'init' => check_plain($name),
      'access' => time(),
      'picture' => $renren_profile->tiny,
    );
  $user = user_save(NULL, $user_edit);
  
  if ($user->uid) {
    renren_add_mapping($user, $renren_profile);
    drupal_set_message(t('Added drupal account with renren profile. You must login and change it.'));
    return $user;
  }
  return NULL;
}

/**
 * Add mapping for drupal with Renren
 */
function renren_add_mapping($drupal_user = '', $renren_profile) {
  if (empty($drupal_user) && user_is_logged_in()) {
    global $user;
    $drupal_user = $user;
  }
  if (!empty($drupal_user)) {
    db_query('INSERT INTO {renren_account_mapping} SET renren_uid = %d, uid = %d, timestamp = %d, name = "%s", tinyurl = "%s"', array($renren_profile->uid, $drupal_user->uid, time(), $renren_profile->name, $renren_profile->tinyurl));
    return TRUE;
  }
  return FALSE;
}