<?php

/**
 * @file
 * dquarks module email component.
 */

/**
 * Implements _dquarks_defaults_component().
 */
function _dquarks_defaults_email() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'mandatory' => 0,
    'extra' => array(
      'width' => '',
      'unique' => 0,
      'disabled' => 0,
      'title_display' => 0,
      'description' => '',
      'attributes' => array(),
      'private' => FALSE,
    ),
  );
}

/**
 * Implements _dquarks_theme_component().
 */
function _dquarks_theme_email() {
  return array(
    'dquarks_display_email' => array(
      'render element' => 'element',
      'file' => 'components/email.inc',
    ),
  );
}

/**
 * Implements _dquarks_edit_component().
 */
function _dquarks_edit_email($component) {
  $form = array();
  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Default value'),
    '#default_value' => $component['value'],
    '#description' => t('The default value of the field.') . theme('dquarks_token_help'),
    '#size' => 60,
    '#maxlength' => 127,
    '#weight' => 0,
    '#attributes' => ($component['value'] == '%useremail' && count(form_get_errors()) == 0) ? array('disabled' => TRUE) : array(),
    '#id' => 'email-value',
  );
  $form['user_email'] = array(
    '#type' => 'checkbox',
    '#title' => t('User email as default'),
    '#default_value' => $component['value'] == '%useremail' ? 1 : 0,
    '#description' => t('Set the default value of this field to the user email, if he/she is logged in.'),
    '#attributes' => array('onclick' => 'getElementById("email-value").value = (this.checked ? "%useremail" : ""); getElementById("email-value").disabled = this.checked;'),
    '#weight' => 0,
    '#element_validate' => array('_dquarks_edit_email_validate'),
  );
  $form['display']['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#default_value' => $component['extra']['width'],
    '#description' => t('Width of the textfield.') . ' ' . t('Leaving blank will use the default size.'),
    '#size' => 5,
    '#maxlength' => 10,
    '#parents' => array('extra', 'width'),
  );
  $form['display']['disabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disabled'),
    '#return_value' => 1,
    '#description' => t('Make this field non-editable. Useful for setting an unchangeable default value.'),
    '#weight' => 11,
    '#default_value' => $component['extra']['disabled'],
    '#parents' => array('extra', 'disabled'),
  );
  $form['validation']['unique'] = array(
    '#type' => 'checkbox',
    '#title' => t('Unique'),
    '#return_value' => 1,
    '#description' => t('Check that all entered values for this field are unique. The same value is not allowed to be used twice.'),
    '#weight' => 1,
    '#default_value' => $component['extra']['unique'],
    '#parents' => array('extra', 'unique'),
  );
  return $form;
}

/**
 * Element validation function for the email edit form.
 */
function _dquarks_edit_email_validate($element, &$form_state) {
  if ($form_state['values']['user_email']) {
    $form_state['values']['value'] = '%useremail';
  }
}

/**
 * Implements _dquarks_render_component().
 */
function _dquarks_render_email($component, $value = NULL, $filter = TRUE) {
  global $user;
  $element = array(
    '#type' => 'textfield',
    '#title' => $filter ? _dquarks_filter_xss($component['name']) : $component['name'],
    '#title_display' => $component['extra']['title_display'] ? $component['extra']['title_display'] : 'before',
    '#default_value' => _dquarks_filter_values($component['value']),
    '#required' => $component['mandatory'],
    '#weight' => $component['weight'],
    '#description' => $filter ? _dquarks_filter_descriptions($component['extra']['description']) : $component['extra']['description'],
    '#attributes' => $component['extra']['attributes'],
    '#element_validate' => array('_dquarks_validate_email'),
    '#theme_wrappers' => array('dquarks_element'),
    '#dquarks_component' => $component,
  );

  // Add an e-mail class for identifying the difference from normal textfields.
  $element['#attributes']['class'][] = 'email';

  // Enforce uniqueness.
  if ($component['extra']['unique']) {
    $element['#element_validate'][] = 'dquarks_validate_unique';
  }

  if (isset($value)) {
    $element['#default_value'] = $value[0];
  }

  if ($component['extra']['disabled']) {
    $element['#attributes']['readonly'] = 'readonly';
  }

  // Change the 'width' option to the correct 'size' option.
  if ($component['extra']['width'] > 0) {
    $element['#size'] = $component['extra']['width'];
  }

  return $element;
}

/**
 * A Drupal Form API Validation function. Validates the entered values from
 * email components on the client-side form.
 *
 * @param array $form_element
 *   The e-mail form element.
 * @param array $form_state
 *   The full form state for the dquarks.
 *
 * @return string
 *   None. Calls a form_set_error if the e-mail is not valid.
 */
function _dquarks_validate_email($form_element, &$form_state) {
  $component = $form_element['#dquarks_component'];
  $value = trim($form_element['#value']);
  if ($value !== '' && !valid_email_address($value)) {
    form_error($form_element, t('%value is not a valid email address.', array('%value' => $value)));
  }
  else {
    form_set_value($form_element, $value, $form_state);
  }
}

/**
 * Implements _dquarks_display_component().
 */
function _dquarks_display_email($component, $value, $format = 'html') {
  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'dquarks_display_email',
    '#theme_wrappers' => $format == 'html' ? array('dquarks_element') : array('dquarks_element_text'),
    '#format' => $format,
    '#value' => isset($value[0]) ? $value[0] : '',
    '#dquarks_component' => $component,
  );
}

/**
 * Format the text output for this component.
 */
function theme_dquarks_display_email($variables) {
  $element = $variables['element'];
  $element['#value'] = empty($element['#value']) ? ' ' : $element['#value'];
  return $element['#format'] == 'html' ? check_plain($element['#value']) : $element['#value'];
}

/**
 * Implements _dquarks_analysis_component().
 */
function _dquarks_analysis_email($component, $sids = array()) {
  $query = db_select('dquarks_submitted_data', 'wsd', array('fetch' => PDO::FETCH_ASSOC))
          ->fields('wsd', array('no', 'data'))
          ->condition('nid', $component['nid'])
          ->condition('cid', $component['cid']);

  if (count($sids)) {
    $query->condition('sid', $sids, 'IN');
  }

  $nonblanks = 0;
  $submissions = 0;
  $wordcount = 0;

  $result = $query->execute();
  foreach ($result as $data) {
    if (drupal_strlen(trim($data['data'])) > 0) {
      $nonblanks++;
      $wordcount += str_word_count(trim($data['data']));
    }
    $submissions++;
  }

  $rows[0] = array(t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array(t('User entered value'), $nonblanks);
  $rows[2] = array(
    t('Average submission length in words (ex blanks)'),
    ($nonblanks != 0 ? number_format($wordcount / $nonblanks, 2) : '0'),
  );
  return $rows;
}

/**
 * Implements _dquarks_table_component().
 */
function _dquarks_table_email($component, $value) {
  return check_plain(empty($value[0]) ? '' : $value[0]);
}

/**
 * Implements _dquarks_csv_headers_component().
 */
function _dquarks_csv_headers_email($component, $export_options) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/**
 * Implements _dquarks_csv_data_component().
 */
function _dquarks_csv_data_email($component, $export_options, $value) {
  return empty($value[0]) ? '' : $value[0];
}

/**
 * Calculate and returns statistics about results for this component from all
 * submission to this dquarks. The output of this function will be displayed
 * under the "results" tab then "analysis".
 *
 * @param array $component
 *   An array of information describing the component, directly correlating to
 *   the dquarks_component database schema
 * @param array $sids
 *   An optional array of submission IDs (sid). If supplied, the analysis will
 *   be limited
 *   to these sids.
 *
 * @return array
 *   An array of data rows, each containing a statistic for this component's
 *   submissions.
 */
function _dquarks_analysis_rows_email($component, $sids = array()) {
  $sidfilter = count($sids) ? " AND sid IN :sids " : "";
  $result = db_query('SELECT data FROM {dquarks_submitted_data} WHERE nid = :nid AND cid = :cid' . $sidfilter, array(
    ':nid' => $component['nid'],
    ':cid' => $component['cid'],
    ':sids' => $sids)
  );

  $nonblanks = 0;
  $submissions = 0;
  $wordcount = 0;
  $rows = array();

  foreach ($result as $data) {
    if (drupal_strlen(trim($data->data)) > 0) {
      $nonblanks++;
      $wordcount += str_word_count(trim($data->data));
    }
    $submissions++;
  }

  $rows[0] = array(t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array(t('User entered value'), $nonblanks);
  $rows[2] = array(
    t('Average submission length in words (ex blanks)'),
    ($nonblanks != 0 ? number_format($wordcount / $nonblanks, 2) : '0'),
  );
  return $rows;
}

/**
 * Return the result of this component's submission for display in a table. The
 * output of this function will be displayed under the "results" tab then
 * "table".
 *
 * @param array $data
 *   An array of information containing the submission result, directly
 *   correlating to the dquarks_submitted_data database schema
 *
 * @return string
 *   Textual output formatted for human reading.
 */
function _dquarks_table_data_email($data) {
  return check_plain(empty($data['value']['0']) ? '' : $data['value']['0']);
}
