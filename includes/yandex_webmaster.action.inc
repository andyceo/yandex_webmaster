<?php

/**
 * @file
 * Action functions for Yandex.Webmaster module.
 *
 * @author Andrey Andreev aka andyceo, <andyceo@yandex.ru>, http://andyceo.ruware.com/
 */

function yandex_webmaster_add_form($form, &$form_state) {
  global $base_url;
  $default = parse_url($base_url);
  $default = $default['host'];
  $form['host'] = array(
    '#type' => 'textfield',
    '#title' => t('Host to add to Yandex.Webmaster'),
    '#default_value' => $default,
    '#disabled' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add host'),
  );
  return $form;
}