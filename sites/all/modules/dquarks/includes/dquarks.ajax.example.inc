<?php

/**
 * @file
 * @category Advertizing.
 * @package dquarks
 * @subpackage QuizzNotification
 * @name QuizzNotification
 *
 * @author IP-TECH
 */

/**
 * Implements hook_form().
 *
 * @param Array $form
 *   An array of form item of notification parameter.
 * @param Array $form_state
 *   An array of form item values of notification parameter.
 * @param object $node
 *   The node id.
 */
function dquarks_ajax_example_form($form, &$form_state, $node) {
  $form = array();
  $items = array();
  $form['items'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
    '#weight' => -2,
    '#theme' => 'dquarks_edit_multichoice_items',
    '#prefix' => "<div id='all_questions'>",
    '#suffix' => "</div>",
  );
  if (isset($form_state['values'])) {
    $items = $form_state['values']['items'];


    $ajax_path = "node/" . $node->nid . "/dquarks/components";
    foreach ($items["notes"] as $key => $option) {
      $form['items']["choices"][$key] = array(
        '#type' => 'checkbox',
        '#default_value' => isset($items["choices"][$key]) ? $items["choices"][$key] : FALSE,
        '#parents' => array('items', 'choices', $key),
      );
      $form['items']["labels"][$key] = array(
        '#type' => 'textfield',
        '#size' => 30,
        '#default_value' => isset($items["labels"][$key]) ? $items["labels"][$key] : '',
        '#parents' => array('items', 'labels', $key),
      );
      $form['items']["notes"][$key] = array(
        '#type' => 'textfield',
        '#size' => 30,
        '#default_value' => isset($items["notes"][$key]) ? $option : 0,
        '#parents' => array('items', 'notes', $key),
      );
      $form['items']["operations"][$key] = array(
        '#type' => 'submit',
        '#value' => t('Delete'),
        '#submit' => array("_dquarks_edit_submit_ajax_example_del_questions"),
        '#parents' => array('items', 'operations', $key),
        '#ajax' => array(
          'path' => $ajax_path . "/callback/" . $key,
          'callback' => '_dquarks_edit_submit_ajax_example_questions_ajax_callback',
          'wrapper' => 'all_questions',
          'effect' => 'fade',
          'method' => 'replace',
        ),
      );
    }
  }

  $form['items']["choices"]['add'] = array(
    '#type' => 'checkbox',
    '#parents' => array('items', 'choices', 'add'),
  );
  $form['items']["labels"]['add'] = array(
    '#type' => 'textfield',
    '#size' => 30,
    '#parents' => array('items', 'labels', 'add'),
  );
  $form['items']["notes"]['add'] = array(
    '#type' => 'textfield',
    '#size' => 30,
    '#default_value' => 0,
    '#parents' => array('items', 'notes', 'add'),
  );
  $form['items']["operations"]['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
    '#submit' => array('dquarks_edit_submit_ajax_example_add_questions'),
    '#parents' => array('items', 'operations', 'add'),
    '#ajax' => array(
      'callback' => '_dquarks_edit_submit_ajax_example_questions_ajax_callback',
      'wrapper' => 'all_questions',
      'effect' => 'fade',
    ),
  );
  
$form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Notify'),
    '#weight' => 1,
  );
  return $form;
}

/**
 * Submit callback function to delete a choice / question
 *
 * @param array $form
 *   An array of form items of multichoice component.
 * @param array $form_state
 *   An array of form items of multichoice component.
 */
function _dquarks_edit_submit_ajax_example_del_questions($form, &$form_state) {
  $items = array("choices", "labels", "notes");
  $id = (int) check_plain($_POST['del_question']);
  foreach ($items as $keys) {
    unset($form_state['input']['items'][$keys][$id]);
    unset($form_state['values']['items'][$keys][$id]);
    unset($form['items'][$keys][$id]);
  }
  unset($form_state['values']['items']['operations'][$id]);
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback to update an option fieldset.
 *
 * @param arrray $form
 *   An array of form items of multichoice component.
 * @param array $form_state
 *   An array of form items of multichoice component.
 *
 * @return array
 *   Return the option fieldset after update.
 */
function _dquarks_edit_submit_ajax_example_questions_ajax_callback($form, $form_state) {
  return $form['items'];
}

/**
 *  Add choice in the option fieldset
 *
 * @param array $form
 *   An array of form items of multichoice component.
 * @param array $form_state
 *   An array of form items values of multichoice component.
 */
function dquarks_edit_submit_ajax_example_add_questions($form, &$form_state) {
  $items = array('choices', 'labels', 'notes');
  foreach ($items as $keys) {
    $add[$keys] = $form_state['input']['items'][$keys]['add'];
    unset($form_state['input']['items'][$keys]['add']);
  }
  if (count($form_state['values']['items']['choices']) > 1) {
    $max = 1 + max(array_keys($form_state['values']['items']['choices']));
  }
  else {
    $max = 1 + array_pop(array_keys($form_state['values']['items']['choices']));
  }
  if (!empty($add['labels'])) {
    foreach ($items as $keys) {
      $form_state['values']['items'][$keys][$max] = $add[$keys];
      unset($form_state['values']['items'][$keys]['add']);
    }
  }
  $form_state['rebuild'] = TRUE;
}

