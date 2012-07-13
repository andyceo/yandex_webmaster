<?php

/**
 * @file
 * Admin-related functions.
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

function yandex_webmaster_add_callback() {
  $o = 'Add Site';
  return $o;
}


function yandex_webmaster_admin_form($form, &$form_state) {
  global $user;
  $cache = db_select('yandex_webmaster', 'yw')
    ->fields('yw')
    ->condition('uid', $user->uid)
    ->execute()
    ->fetchObject();

  if (!$cache || !isset($cache->expired) || ($cache->expired < time())) {
    $attributes = array(
      'onclick' => '',
      'target' => '_blank',
    );
    $auth_status = l(t('Allow module gather information from Yandex.Webmaster.'), YANDEX_WEBMASTER_OAUTH_LINK, array('attributes' => $attributes));
    $auth_code = str_repeat('0', 7);

    $form['auth_text'] = array(
      '#type' => 'markup',
      '#markup' => $auth_status,
    );
    $form['authorization_code'] = array(
      '#type' => 'textfield',
      '#title' => t('Confirmation code'),
      '#description' => t('Enter code you obtain from Yandex.Webmaster.'),
      '#default_value' => $auth_code,
      '#disabled' => (bool)(int)$auth_code,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#disabled' => (bool)(int)$auth_code,
    );
  }
  else {
    $auth_status = t('Module Yandex.Webmaster has a permission to gather information about your sites. This permission is valid until @expired', array('@expired' => format_date($cache->expired)));
    $auth_status .= '<br />' . t('Your authentication code is %code', array('%code' => $cache->auth_code));
    $auth_status .= '<br />' . t('Your token is %token', array('%token' => $cache->token));

    $form['auth_text'] = array(
      '#type' => 'markup',
      '#markup' => $auth_status,
    );
  }

  // Declare auth form for 1 case (not registerd)

  $form['step1'] = array(
      '#type' => 'fieldset',
      '#title' => t('Step 1: Get authorization code'),
      //'#description' => t('Enter code you obtain from Yandex.Webmaster.'),
      //'#default_value' => $auth_code,
      //'#disabled' => (bool)(int)$auth_code,
    );

  $form['step2'] = array(
      '#type' => 'fieldset',
      '#title' => t('Step 2: Enter authorization code'),
      //'#description' => t('Enter code you obtain from Yandex.Webmaster.'),
      //'#default_value' => $auth_code,
      //'#disabled' => (bool)(int)$auth_code,
    );

  $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings'),
      //'#description' => t('Enter code you obtain from Yandex.Webmaster.'),
      //'#default_value' => $auth_code,
      //'#disabled' => (bool)(int)$auth_code,
    );

  return $form;
}

function yandex_webmaster_admin_form_validate($form, &$form_state) {
  if (!$token = yandex_webmaster_authenthicate($form_state['values']['authorization_code'])) {
    form_set_error('authorization_code', t('Wrong authorization code %code! Can not get OAuth token.', array('%code' => $form_state['values']['authorization_code'])));
  }
  // On success, token was already set in yandex_webmaster_authenthicate(), so we don't need submit handler.
}

