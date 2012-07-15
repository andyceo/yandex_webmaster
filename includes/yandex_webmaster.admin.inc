<?php

/**
 * @file
 * Admin-related functions.
 *
 * @author Andrey Andreev aka andyceo, <andyceo@yandex.ru>, http://andyceo.ruware.com/
 */

function yandex_webmaster_auth_form($form, &$form_state) {
  // Declare 2 fieldsets
  $form['step1'] = array(
    '#type' => 'fieldset',
    '#title' => t('Step 1: Get confirmation code'),
  );
  $form['step2'] = array(
    '#type' => 'fieldset',
    '#title' => t('Step 2: Enter confirmation code'),
  );

  // Generate OAuth link and default value for auth code
  $attributes = array(
    'onclick' => '',
    'target' => '_blank',
  );
  $auth_link = l(t('Allow module gather information from Yandex.Webmaster.'), YANDEX_WEBMASTER_OAUTH_LINK, array('attributes' => $attributes));

  // Fill the fieldsets with form data.
  $form['step1']['auth_text'] = array(
    '#type' => 'markup',
    '#markup' => $auth_link,
  );

  $form['step2']['authorization_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Confirmation code'),
    '#description' => t('Enter the code you obtain from Yandex.Webmaster.'),
    '#default_value' => str_repeat('0', 7),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  // Check if the user already has the token an it is not expired.
  global $user;
  $cache = db_select('yandex_webmaster', 'yw')
    ->fields('yw')
    ->condition('uid', $user->uid)
    ->execute()
    ->fetchObject();
  if ($cache && isset($cache->auth_code)) {
    // Update the Step 2 fieldset
    $form['step2']['authorization_code']['#default_value'] = $cache->auth_code;

    if (isset($cache->expired) && ($cache->expired > time())) {
      // User already has a token, but he (she) can obtain the new one.

      // Update the Step 1 fieldset
      $form['step1']['auth_text']['#markup'] = '<p>' . t('Module Yandex.Webmaster has a permission to gather information about your sites. This permission is valid until @expired', array('@expired' => format_date($cache->expired))) . '</p>';
      $form['step1']['auth_text']['#markup'] .= '<p>' . t('If you want to obtain new token, please use the link below.') . '</p>';
      $form['step1']['auth_text']['#markup'] .= $auth_link;
    }
    else {
      // Expired token. Get the new one
      $form['step1']['auth_text']['#markup'] = '<p>' . t('Your token was expired. Please use link below to obtain the new one.') . '</p>';
      $form['step1']['auth_text']['#markup'] .= $auth_link;
    }
  }

  return $form;
}

function yandex_webmaster_auth_form_validate($form, &$form_state) {
  if ($form_state['values']['authorization_code'] != $form['step2']['authorization_code']['#default_value']) {
    if (!$token = yandex_webmaster_authenthicate($form_state['values']['authorization_code'])) {
      form_set_error('authorization_code', t('Wrong authorization code %code! Can not get OAuth token.', array('%code' => $form_state['values']['authorization_code'])));
    }
    // On success, token was already set in yandex_webmaster_authenthicate(), so we don't need submit handler.
  }
}

function yandex_webmaster_settings_form($form, &$form_state) {
  // Timeout
  $form['timeout'] = array(
    '#type' => 'select',
    '#title' => t('Timeout'),
    '#description' => t('Set the connection timeout for Yandex.Webmaster service.'),
    '#options' => array(
      '5' => t('5 sec'),
      '7' => t('7 sec'),
      '10' => t('10 sec'),
      '15' => t('15 sec'),
      '20' => t('20 sec'),
    ),
    '#default_value' => variable_get('yandex_webmaster_timeout', YANDEX_WEBMASTER_DEFAULT_TIMEOUT),
    '#access' => user_access('administer site configuration'),
  );

  // Cache lifetime
  $form['cache_lifetime'] = array(
    '#type' => 'textfield',
    '#title' => t('Cache lifetime'),
    '#description' => t('Set the Yandex.Webmaster cache lifetime.'),
    '#default_value' => variable_get('yandex_webmaster_cache_lifetime', YANDEX_WEBMASTER_DEFAULT_CACHE_LIFETIME),
    '#access' => user_access('administer site configuration'),
  );

  // Select language and domain name.
  $form['domain'] = array(
    '#type' => 'select',
    '#title' => t('Language domain'),
    '#description' => t('Select Yandex.Webmaster language domain'),
    '#options' => array(
      'webmaster.yandex.com' => t('English'),
      'webmaster.yandex.ru' => t('Russian'),
      'webmaster.yandex.com.tr' => t('Turkish'),
      'webmaster.yandex.ua' => t('Ukrainian'),
    ),
    '#default_value' => 'webmaster.yandex.com',
  );

  // Detect the Yandex.Webmaster language domain based on user language
  // and site default language.
  global $user, $language;
  foreach (array_keys($form['domain']['#options']) as $domain) {
    if (strpos($domain, $user->language)) {
      $form['domain']['#default_value'] = $domain;
      break;
    }
    elseif(strpos($domain, $language->language)) {
      $form['domain']['#default_value'] = $domain;
      break;
    }
  }

  return $form;
}
