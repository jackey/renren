<?php
/**
 * @file
 */

// $Id$

/**
 * Setting page for renren connection variable: API and Secret
 * @param $form_state
 */
function renren_connection_settings(&$form_state) {
  $forms = array();
  $forms['renren_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Renren API settings'),
    '#description' => t('Please config API for renren if you want to connect renren server'),
  );
  // renren api settings
  $forms['renren_settings']['renren_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#default_value' => variable_get('renren_api_key', ''),
    '#description' => t('You should input the api key to access renren sever. You can get it from ' . l(t('here'), 'www.dev.renren.com', array('absolute' => TRUE))),
  );
  $forms['renren_settings']['renren_api_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('API Secret'),
    '#default_value' => variable_get('renren_api_secret', ''),
    '#description' => t('Please input the API Secret. You should have it for accessing the user data'),
  );
  return system_settings_form($forms);
}

/**
 * Settings page for Renren action template
 */
function renren_action_template_settings() {
  $forms = array(
    'renren_template_comment_title' => array(
      '#title' => t('The comment title will be display at renren action'),
      '#type' => 'textfield',
      '#default_value' => variable_get('renren_template_comment_title', ''),
      '#description' => t('If you want to the comment title be display at renren action,
                          you should set action template pattern for it. Example: title'),
    ),
    'renren_template_comment_body' => array(
      '#title' => t('The comment body will be displayed at renren action'),
      '#type' => 'textfield',
      '#default_value' => variable_get('renren_template_comment_body', ''),
      '#description' => t('If you want to the comment body be displayed at renren action,
                           you should set action template pattern for it. Example: body'),
    ),
  );
  return system_settings_form($forms);
}