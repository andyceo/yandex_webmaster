<?php

/**
 * @file
 * Contains Yandex.Webmaster class definition.
 *
 * @author Andrey Andreev aka andyceo, <andyceo@yandex.ru>, http://andyceo.ruware.com/
 */

require_once('yandex_webmaster.parser.inc');

/**
 * Define a Drupal-related class to work with Yandex.Webmaster service.
 */
class DrupalYandexWebmaster {

  /**
   * OAuth token for access to Yandex.Webmaster.
   */
  protected $token;

  /**
   * Yandex.Webmaster API url.
   */
  protected $url;

  /**
   * Query timeout, in seconds.
   */
  protected $timeout = 5;

  /**
   * Yandex.Webmaster User ID.
   */
  protected $uid;

  protected $host_list_url;
  protected $host_list;

  // Error handling variables
  const UNKNOWN_ERROR_CODE = -42;
  protected $error_code; // int | string
  protected $error_message; //string


  /**
   * Constructor fully Drupal-specific.
   *
   * @param $token
   * @param $url
   * @param $cache
   */
  function __construct($token, $url, $cache = NULL) {
    $this->token = $token;
    $this->url = $url;

    if (!$cache) {
      $cache = $this->getServiceInfo();
    }
    if (is_array($cache)) {
      $cache = (object)$cache;
    }
    if (is_object($cache)) {
      $this->uid = $cache->uid;
      $this->host_list_url = $cache->host_list_url;
      $this->host_list = $cache->host_list;
    }
  }

  function setTimeout($timeout) {
    if (is_numeric($timeout)) {
      $this->timeout = $timeout;
    }
  }

  function getServiceInfo() {
    if (!isset($this->host_list) || !is_array($this->host_list) || !count($this->host_list)) {
      $this->getHostList();
    }
    foreach ($this->host_list as $hid => $host_resources) {
      $this->host_list[$hid] = $this->getHostResources($hid);
    }
    return (object) array(
      'uid' => $this->getUid(),
      'host_list_url' => $this->getHostListUrl(),
      'host_list' => $this->host_list,
    );
  }

  function getLastError() {
    return (object)array(
      'code' => $this->error_code,
      'message' => $this->error_message,
    );
  }

////////////////////API METHODS////////////////////
// @see http://api.yandex.ru/webmaster/doc/dg/concepts/getting-started.xml - API overview.

  /**
   * Return the User ID.
   *
   * @see http://api.yandex.ru/webmaster/doc/dg/reference/getting-uid.xml
   */
  function getUid() {
    if (isset($this->uid) && is_numeric($this->uid)) {
      return $this->uid;
    }

    // We will recieve a redirect to service document with that settings
    // and it will be automatically parsed in $this->request();
    $url = $this->url . '/me';
    $options = array('max_redirects' => 1); // we want to get a service document at once

    if (!$result = $this->request($url, $options, 'DrupalYandexWebmasterXMLParserServiceDocument')) {
      return FALSE;
    }

    $this->host_list_url = $result;

    // After redirection, we have our user id in $result['redirect_url']
    $result = str_replace('/hosts', '', $result);
    $last_slash_pos = strrpos($result, '/');
    $this->uid = (int)drupal_substr($result, $last_slash_pos + 1);

    return $this->uid;
  }

  /**
   * Retrive Yandex.Webmaster API service document and return the
   * Yandex.Webmaster host list url.
   */
  function getHostListUrl() {
    if (isset($this->host_list_url) && !empty($this->host_list_url)) {
      return $this->host_list_url;
    }
    if ($this->getUid()) {
      return $this->host_list_url;
    }
    return FALSE;
  }

  function getHostList() {
    if (!$host_list_url = $this->getHostListUrl()) {
      return FALSE;
    }

    if (!$result = $this->request($host_list_url, array(), 'DrupalYandexWebmasterXMLParserHostList')) {
      return FALSE;
    }

    foreach ($result as &$host) {
      $last_slash_pos = strrpos($host['href'], '/');
      $hid = (int)drupal_substr($host['href'], $last_slash_pos + 1);
      $host['hid'] = $hid;
      $this->host_list[$hid] = array();
    }

    return $result;
  }

  function getHostResources($hid) {
    if (isset($this->host_list[$hid]) && is_array($this->host_list[$hid]) && count($this->host_list[$hid])) {
      return $this->host_list[$hid];
    }
    $url = $this->getHostListUrl() . '/' . $hid;
    if (!$result = $this->request($url, array(), 'DrupalYandexWebmasterXMLParserHostResources')) {
      return FALSE;
    }
    foreach ($result as $resource_link) {
      $key = str_replace($url . '/', '', $resource_link);
      $this->host_list[$hid][$key] = $key;
    }
    return $this->host_list[$hid];
  }

  function getHostStatistics($hid) {
    return $this->getHostAPIRequest($hid, 'stats', 'DrupalYandexWebmasterXMLParserHostStatistics');
  }

  function getHostVerifiedStatus($hid) {
    return $this->getHostAPIRequest($hid, 'verify', 'DrupalYandexWebmasterXMLParserHostVerified');
  }

  function getHostExcludedPages($hid) {
    return $this->getHostAPIRequest($hid, 'excluded', 'DrupalYandexWebmasterXMLParserHostExcluded');
  }

  function getHostIndexedPages($hid) {
    return $this->getHostAPIRequest($hid, 'indexed', 'DrupalYandexWebmasterXMLParserHostIndexed');
  }

  function getHostTopQueries($hid) {
    return $this->getHostAPIRequest($hid, 'tops', 'DrupalYandexWebmasterXMLParserTopQueries');
  }

  function getHostExternalLinks($hid) {
    return $this->getHostAPIRequest($hid, 'links', 'DrupalYandexWebmasterXMLParserExternalLinks');
  }

////////////////END OF API METHODS////////////////

  /**
   * This is a helper function. It make easier to do similar requests.
   */
  private function getHostAPIRequest($hid, $key, $parser) {
    if ((!$resources = $this->getHostResources($hid)) || !isset($resources[$key])) {
      return FALSE;
    }
    $url = $this->getHostListUrl() . '/' . $hid . '/' . $resources[$key];
    if (!$result = $this->request($url, array(), $parser)) {
      return FALSE;
    }
    return $result;
  }

  /**
   * Analogue of drupal_http_request($url, $options).
   */
  private function request($url, $options, $parser) {
    // Add token to the OAuth headers
    if (isset($options['headers']) && is_array($options['headers'])) {
      $options['headers']['Authorization'] = 'OAuth ' . $this->token;
    }
    else {
      $options = array(
        'headers' => array(
          'Authorization' => 'OAuth ' . $this->token,
        ),
      );
    }

    // Set preferred timeout
    if (!isset($options['timeout'])) {
      $options['timeout'] = $this->timeout;
    }

    // add statistics marker to the query
    //global $language;
    // We can guarantee that in our queries there is no query part and
    // fragment part in $url, so we don't need to check for that.
    //$url .= '?wizard=drupal_yandex_webmaster_' . $language->language;

    // execute query
    $result = drupal_http_request($url, $options);

    // Now, let check for errors.

    // Check for timeout
    if ($result->code == HTTP_REQUEST_TIMEOUT) {
      $this->error_code = $this->http_result->code;
      $this->error_message = $result->error;
      return FALSE;
    }

    // Check for HTTP errors
    if (isset($this->http_result->error)) {
      $this->error_type = 'http';
      $this->error_code = $this->http_result->code;
      $this->error_message = $this->http_result->error;
      return FALSE;
    }

    $parser = new $parser;
    if (!$parser->parse($result->data)) {
      $error = $parser->error();
      $this->error_code = $error->code;
      $this->error_message = "Can't parse xml. Error in line " . $error->line . " with message: " . $error->message;
      return FALSE;
    }

    return $parser->data;
  }
}
