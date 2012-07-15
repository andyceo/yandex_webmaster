<?php

/**
 * @file
 * Report-related functions for Yandex.Webmaster module.
 *
 * @author Andrey Andreev aka andyceo, <andyceo@yandex.ru>, http://andyceo.ruware.com/
 */


function yandex_webmaster_list_callback() {
  $table = array(
    'header' => array(
      t('Site'),
      t('TCY'),
      t('Loaded pages'),
      t('Indexed pages'),
      t('Last access'),
      t('Virused'),
      t('Crawling'),
      t('Verified'),
      t('Delete'),
    ),
    'rows' => array(),
    'empty' => t('No hosts found.'),
  );

  $column_count = count($table['header']);

  if (($yw = yandex_webmaster_instance()) && ($host_list = $yw->getHostList()) && count($host_list)) {
    foreach ($host_list as $host) {
      $row = array($host['name']);
      if (!isset($host['verification']) || $host['verification']['state'] != 'VERIFIED') {
        $row[] = array(
          'data' => t('Host not verified'),
          'colspan' => $column_count - 1,
        );
      }
      else {
        $row[] = $host['tcy'];
        $row[] = $host['url-count'];
        $row[] = $host['index-count'];
        $row[] = $host['last-access'];
        $row[] = $host['virused'] ? t('Yes') : t('No');
        $row[] = $host['crawling']['state'] == 'INDEXED' ? t('Yes') : t('No');
        $row[] = $host['verification']['state'] == 'VERIFIED' ? t('Yes') : t('No');
        $row[] = t('Delete');
      }
      $table['rows'][] = $row;
    }
  }
  else {
    drupal_set_message(t("Can't get host list from Yandex.Webmaster."), 'error');
  }

  return theme('table', $table);
}

/**
 * Menu callback for host statistics.
 */
function yandex_webmaster_stats_callback() {
  list($hid, $yw, $host_properties, $filter, $wrapper) = _yandex_webmaster_reports_helper(__FUNCTION__);

  $stats = array();
  if (!empty($yw)) {
    $stats = $yw->getHostStatistics($hid);
  }

  $host_table = array(
    '#theme' => 'table',
    '#header' => array(t('Indicator'), t('Value')),
    '#rows' => array(),
    '#empty' => t('No host information available.'),
  );

  foreach ($host_properties as $key => $property) {
    $host_table['#rows'][] = array($property, $stats[$key]);
  }

  $wrapper[__FUNCTION__ . '_table'] = $host_table;

  $page = array(
    'yandex_webmaster_report_filter_form' => $filter,
    'yandex_webmaster_table' => $wrapper,
  );

  return $page;
}

/**
 * Menu callback for host statistics.
 */
function yandex_webmaster_indexed_callback() {
  list($hid, $yw, $host_properties, $filter, $wrapper) = _yandex_webmaster_reports_helper(__FUNCTION__);

  if (!empty($yw)) {
    $indexed = $yw->getHostIndexedPages($hid);
  }

  $url_list_table = array(
    '#theme' => 'table',
    '#header' => array(t('URL')),
    '#rows' => array(),
    '#empty' => t('No URLs.'),
    '#caption' => $host_properties['index-count'] . ': ' . $indexed['count'],
  );

  if (isset($indexed['urls']) && is_array($indexed['urls']) && count($indexed['urls'])) {
    foreach ($indexed['urls'] as $row) {
      $url_list_table['#rows'][] = array($row);
    }
  }

  $wrapper[__FUNCTION__ . '_table'] = $url_list_table;

  $page = array(
    'yandex_webmaster_report_filter_form' => $filter,
    'yandex_webmaster_table' => $wrapper,
  );

  return $page;
}

/**
 * Menu callback for host excluded pages.
 */
function yandex_webmaster_excluded_callback() {
  list($hid, $yw, $host_properties, $filter, $wrapper) = _yandex_webmaster_reports_helper(__FUNCTION__);

  if (!empty($yw)) {
    $excluded = $yw->getHostExcludedPages($hid);
  }

  $url_list_table = array(
    '#theme' => 'table',
    '#header' => array(t('URL')),
    '#rows' => array(),
    '#empty' => t('No URLs.'),
    '#caption' => $host_properties['url-errors'] . ': ' . $excluded['count'],
  );

  if (isset($excluded['urls']) && is_array($excluded['urls']) && count($excluded['urls'])) {
    foreach ($excluded['urls'] as $row) {
      $host_table['rows'][] = $row;
    }
  }

  $wrapper[__FUNCTION__ . '_table'] = $url_list_table;

  $page = array(
    'yandex_webmaster_report_filter_form' => $filter,
    'yandex_webmaster_table' => $wrapper,
  );

  return $page;
}

/**
 * Menu callback for host statistics.
 */
function yandex_webmaster_tops_callback() {
  list($hid, $yw, $host_properties, $filter, $wrapper) = _yandex_webmaster_reports_helper(__FUNCTION__);

  if (!empty($yw)) {
    $top_queries = $yw->getHostTopQueries($hid);
  }

  $table = array(
    '#theme' => 'table',
    '#header' => array(t('Query'), t('Count'), t('Position')),
    '#rows' => array(),
    '#empty' => t('No queries.'),
  );

  $tables = array();
  foreach ($top_queries as $key => $top) {
    $tables[$key] = $table;
    if (isset($top['queries']) && count($top['queries'])) {
      foreach ($top['queries'] as $row) {
        $tables[$key]['#rows'][] = array($row['query'], $row['count'], $row['position']);
      }
    }
  }

  $table = array(
    '#theme' => 'table',
    '#header' => array(
      t('Shows (count: @c, percent: @p)', array('@c' => $top_queries['shows']['count'], '@p' => round($top_queries['shows']['percent'], 2))),
      t('Clicks (count: @c, percent: @p)', array('@c' => $top_queries['shows']['count'], '@p' => round($top_queries['shows']['percent'], 2))),
    ),
    '#rows' => array(array(
      render($tables['shows']),
      render($tables['clicks']),
    )),
    '#empty' => t('No data.'),
  );

  $wrapper[__FUNCTION__ . '_table'] = $table;

  $page = array(
    'yandex_webmaster_report_filter_form' => $filter,
    'yandex_webmaster_table' => $wrapper,
  );

  return $page;
}

/**
 * Menu callback for host statistics.
 */
function yandex_webmaster_links_callback() {
  return t('Currently not supported. You should ask Yandex.Webmaster to get permission for viewing external links for this module.');
}

function yandex_webmaster_report_filter_form($form, &$form_state, $callback) {
  $yw = yandex_webmaster_instance();

  $host_list = array();
  if ($yw) {
    global $user;
    $host_list = db_select('yandex_webmaster_host_list', 'ywhl')
      ->fields('ywhl', array('hid', 'name'))
      ->condition('uid', $user->uid)
      ->orderBy('name')
      ->execute()
      ->fetchAllKeyed();
  }

  $form = array();

  if (empty($host_list)) {
    return $form;
  }

  $form_state['storage']['yandex_webmaster']['callback'] = $callback;

  $form['filter'] = array(
    '#type' => 'fieldset',
  );

  $form['filter']['host'] = array(
    '#title' => t('Host to review'),
    '#type' => 'select',
    '#options' => $host_list,
    '#ajax' => array(
      'callback' => 'yandex_webmaster_update_table_ajax',
      'method' => 'replace',
      'wrapper' => 'yandex_webmaster_table',
    ),
  );
  if (!empty($_SESSION['yandex_webmaster_hid'])) {
    // Get current site from filter
    $form['filter']['host']['#default_value'] = $_SESSION['yandex_webmaster_hid'];
  }
  else {
    // Get first host from host list
    $form['filter']['host']['#default_value'] = array_shift(array_keys($host_list));
  }

  $form['filter']['reset_cache'] = array(
    '#title' => t('Reset cache'),
    '#type' => 'checkbox',
  );

  $form['filter']['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('container-inline')),
  );

  $form['filter']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Show'),
  );

  $form['#attached']['css'][] = drupal_get_path('module', 'yandex_webmaster') . '/css/yandex_webmaster_filter.css';
  $form['#prefix'] = '<div id="yandex_webmaster_filter">';
  $form['#suffix'] = '</div>';

  return $form;
}

function yandex_webmaster_report_filter_form_submit($form, &$form_state) {
  $_SESSION['yandex_webmaster_hid'] = $form_state['values']['host'];
  return drupal_get_destination();
}

/**
 * 'restripe' rows callback.
 *
 * Rebuilds the table with the selected number of rows.
 */
function yandex_webmaster_update_table_ajax($form, &$form_state) {
  $page = array('yandex_webmaster_table' => '<div>' . t('Ajax error') . '</div>');
  if (isset($form_state['storage']['yandex_webmaster']['callback'])) {
    $_SESSION['yandex_webmaster_hid'] = $form_state['values']['host'];
    $page = call_user_func($form_state['storage']['yandex_webmaster']['callback']);
    if (!empty($page) && isset($page['yandex_webmaster_table'])) {
      $page = array('yandex_webmaster_table' => $page['yandex_webmaster_table']);
    }
  }
  return render($page);
}

/**
 * Helper function for report callbacks.
 */
function _yandex_webmaster_reports_helper($callback) {
  $hid = array();
  $hid = yandex_webmaster_report_filter_form($hid, $hid, $callback);
  $hid = $hid['filter']['host']['#default_value'];

  $yw = yandex_webmaster_instance();

  $host_properties = yandex_webmaster_host_properties();

  $filter = drupal_get_form('yandex_webmaster_report_filter_form', $callback);

  $wrapper = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('yandex_webmaster_table'),
      'id' => 'yandex_webmaster_table',
    ),
  );

  return array($hid, $yw, $host_properties, $filter, $wrapper);
}
