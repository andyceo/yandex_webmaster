<?php

/**
 * @file
 * Report-related functions for Yandex.Webmaster module.
 *
 * @author Andrey Andreev aka andyceo, <andyceo@yandex.ru>, http://andyceo.ruware.com/
 */

/**
 * Menu callback for host statistics.
 */
function yandex_webmaster_stats_callback() {
  list($hid, $yw, $host_properties, $filter) = _yandex_webmaster_reports_helper();
  $stats = $yw->getHostStatistics($hid);

  $host_table = array(
    '#theme' => 'table',
    '#header' => array(t('Indicator'), t('Value')),
    '#rows' => array(),
    '#empty' => t('No host information available.'),
  );

  foreach ($host_properties as $key => $p) {
    $host_table['rows'][] = array($p, $stats[$key]);
  }

  $page = array(
    'yandex_webmaster_report_filter_form' => $filter,
    'host_table' => $host_table,
  );

  return $page;
}

/**
 * Menu callback for host statistics.
 */
function yandex_webmaster_indexed_callback() {
  list($hid, $yw, $host_properties, $filter) = _yandex_webmaster_reports_helper();
  $indexed = $yw->getHostIndexedPages($hid);

  $url_list_table = array(
    '#theme' => 'table',
    '#header' => array(t('URL')),
    '#rows' => array(),
    '#empty' => t('No URLs.'),
  );

  if (isset($indexed['indexed']['urls']) && is_array($indexed['indexed']['urls']) && count($indexed['indexed']['urls'])) {
    $host_table['rows'][] = $indexed['urls'];
  }

  $page = array(
    'yandex_webmaster_report_filter_form' => $filter,
    'index-count' => array('#markup' => $host_properties['index-count'] . ': ' . $indexed['count']),
    'url_list_table' => $url_list_table,
  );

  return $page;
}

/**
 * Menu callback for host excluded pages.
 */
function yandex_webmaster_excluded_callback() {
  list($hid, $yw, $host_properties, $filter) = _yandex_webmaster_reports_helper();
  $yw = yandex_webmaster_instance();
  $excluded = $yw->getHostExcludedPages($hid);

  $url_list_table = array(
    '#theme' => 'table',
    '#header' => array(t('URL')),
    '#rows' => array(),
    '#empty' => t('No URLs.'),
  );

  if (isset($excluded['urls']) && is_array($excluded['urls']) && count($excluded['urls'])) {
    $host_table['rows'][] = $excluded['urls'];
  }

  $page = array(
    'yandex_webmaster_report_filter_form' => $filter,
    'index-count' => array('#markup' => $host_properties['index-count'] . ': ' . $excluded['count']),
    'url_list_table' => $url_list_table,
  );

  return $page;
}

/**
 * Menu callback for host statistics.
 */
function yandex_webmaster_tops_callback() {
  list($hid, $yw, $host_properties, $filter) = _yandex_webmaster_reports_helper();
  //$indexed = $yw->getHostStatistics($hid);
  //krumo($indexed);

  $page = array(
    'yandex_webmaster_report_filter_form' => $filter,
  );

  return $page;
}

/**
 * Menu callback for host statistics.
 */
function yandex_webmaster_links_callback() {
  list($hid, $yw, $host_properties, $filter) = _yandex_webmaster_reports_helper();
  $yw = yandex_webmaster_instance();
  //$indexed = $yw->getHostStatistics($hid);
  //krumo($indexed);

  $page = array(
    'yandex_webmaster_report_filter_form' => $filter,
  );

  return $page;
}

function yandex_webmaster_report_filter_form() {
  $hosts = yandex_webmaster_user_host_list();

  $form = array();
  if ($hosts) {
    $form['host'] = array(
      '#title' => t('Host to review'),
      '#description' => t('Choose a host to review.'),
      '#type' => 'select',
      '#options' => $hosts,
    );
    if (!empty($_SESSION['yandex_webmaster_hid'])) {
      $form['host']['#default_value'] = $_SESSION['yandex_webmaster_hid'];
    }

    foreach ($hosts as $hid => $host) {
      $form['host']['#options'][$hid] = $hid;
    }

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Show'),
    );
  }

  return $form;
}

function yandex_webmaster_report_filter_form_submit($form, &$form_state) {
  $_SESSION['yandex_webmaster_hid'] = $form_state['values']['host'];
  return drupal_get_destination();
}

function _yandex_webmaster_reports_helper() {
  $hid = NULL;
  if (isset($_SESSION['yandex_webmaster_hid'])) {
    // Get current site from filter
    $hid = $_SESSION['yandex_webmaster_hid'];
  }
  else {
    // Get first host from host list
    $hid = yandex_webmaster_user_host_list();
    $hid = array_pop($hid);
  }
  $yw = yandex_webmaster_instance();
  $host_properties = yandex_webmaster_host_properties();
  $filter = drupal_get_form('yandex_webmaster_report_filter_form');
  return array($hid, $yw, $host_properties, $filter);
}
