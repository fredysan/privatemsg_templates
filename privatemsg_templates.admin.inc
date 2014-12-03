<?php

/**
 * @file
 * Administration menu callbacks for privatemsg_templates.module.
 */


/**
 * Builds row of sending, receiving roles and actions that go with them.
 *
 * @param $details
 *   Details of the row: default values and the unique row number (delta).
 * @param $blacklist
 *   When the functionality has been added, this will allow building actions
 *   based on a whitelist or blacklist. The current code only covers the use
 *   case of a blacklist, where blocking everyone is allowed by default and
 *   rules are exceptions to that. Conversely, a whitelist will disallow
 *   blocking by default and rules will configure roles that are allowed to
 *   block.
 *
 * @return
 *   Part of a form with controls for sending, receiving and actions.
 */
function _privatemsg_templates_actions_form($details, $blacklist = TRUE) {
  $form = array(
    '#tree' => TRUE,
  );
  $delta = $details['delta'];
  // FALSE by default, or if the user has checked the 'Enabled' check box for
  // this row.
  $row_disabled = (isset($details['enabled']) ? !$details['enabled'] : FALSE);

  $form['template_name'] = array(
    '#type' => 'textfield',
    '#attributes' => ($row_disabled ? array('readonly' => array('readonly')) : array()),
    '#required' => TRUE,
    '#default_value' => (isset($details['template_name']) ? $details['template_name'] : ''),
    '#size' => 30,
  );
  $form['template_body'] = array(
    '#type' => 'textarea',
    '#disabled' => $row_disabled,
    '#required' => TRUE,
    '#default_value' => (isset($details['template_body']) ? $details['template_body'] : ''),
  );
  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#default_value' => (isset($details['enabled']) ? $details['enabled'] : TRUE),
  );
  $form['remove'] = array(
    '#type' => 'submit',
    '#submit' => array('privatemsg_templates_remove_submit'),
    '#value' => t('Remove_' . $delta),
    '#attributes' => array('class' => array('remove-template')),
    '#prefix' => '<div id="remove-template-button">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'callback' => 'privatemsg_templates_js',
      'wrapper' => 'templates',
    ),
  );

  return $form;
}

/**
 * Takes an array of settings and filters out anything that's un-needed.
 *
 * Leaving only settings to be saved.
 *
 * @param $settings
 *   The array of settings to filter.
 * @return
 *   Array of settings, ready to be stored in the database.
 * @see privatemsg_templates_settings_submit()
 */
function _privatemsg_templates_settings_filter($settings) {
  // Add-in the names of any settings to be saved into the array below.
  $save_keys = array('template_name', 'template_body', 'enabled');
  $matching = array();
  // Run through each of the keys we want to save, creating a new array.
  // It's not possible to simply check for unwanted values and unset() them as
  // the array is multi-dimensional.
  foreach ($save_keys as $save_key) {
    if (isset($settings[$save_key])) {
      $matching[$save_key] = $settings[$save_key];
    }
  }
  if (count($matching) > 0) {
    return $matching;
  }
  else {
    return array_map('_privatemsg_templates_settings_filter', $settings);
  }
}

/**
 * Menu callback for AHAH handling.
 */
function privatemsg_templates_js($form, $form_state) {
  return drupal_render($form['templates']);
}

/**
 * Submit handler for 'More' button, adds a new action.
 *
 * @see privatemsg_templates_remove_submit()
 */
function privatemsg_templates_more_submit($form, &$form_state) {
  unset($form_state['submit_handlers']);
  form_execute_handlers('submit', $form, $form_state);
  // Get the submitted actions, then put them into a special area of
  // the $form_state.
  $submitted_values = $form_state['values'];
  // Add an empty action.
  $submitted_values['templates'][] = array();
  $form_state['privatemsg_templates'] = $submitted_values;
  // Rebuild the form by passing our $form_state through the
  // privatemsg_templates_settings() builder function.
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for 'Remove' button, removes an action.
 *
 * @see privatemsg_templates_more_submit()
 */
function privatemsg_templates_remove_submit($form, &$form_state) {
  unset($form_state['submit_handlers']);
  form_execute_handlers('submit', $form, $form_state);
  $submitted_values = $form_state['values'];
  // Remove the requested action.
  $delta = $form_state['clicked_button']['#parents'][1];
  unset($submitted_values['templates'][$delta]);
  $form_state['privatemsg_templates'] = $submitted_values;
  $form_state['rebuild'] = TRUE;
}

/**
 * Menu callback for blocked user settings.
 */
function privatemsg_templates_admin_settings($form, &$form_state) {
  // Need to cache form for AHAH, so it can be rebuilt from cache later.
  $form = array(
    '#cache' => TRUE,
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'privatemsg_templates') . '/privatemsg_templates.css',
      ),
    ),
  );

  // Container for just the actions, used for AHAH.
  $form['templates'] = array(
    '#tree' => TRUE,
    '#prefix' => '<div id="templates">',
    '#suffix' => '</div>',
    '#theme' => 'privatemsg_templates_actions',
  );

  // Should we populate the form with data from $form_state or the database?
  if (!isset($form_state['privatemsg_templates']['templates'])) {
    // TODO: Optimizar el manejo de los valores guardados ya que
    // al guardarse en variables aumenta la memoria que drupal
    // usa todo el tiempo
    $templates = variable_get('privatemsg_templates_actions', array());
  }
  else {
    $templates = $form_state['privatemsg_templates']['templates'];
  }
  // Work through each rule, adding it as a new element in
  // $form['block_actions'] ready to be themed later.
  foreach ($templates as $delta => $details) {
    $details['delta'] = $delta;
    $form['templates'][$delta] = _privatemsg_templates_actions_form($details);
  }

  // The magic AHAH callback button that adds more rows.
  $form['privatemsg_templates_more'] = array(
    '#type' => 'submit',
    '#value' => t('Add new template'),
    '#weight' => 1,
    '#prefix' => '<div id="add-template-button">',
    '#suffix' => '</div>',
    '#submit' => array('privatemsg_templates_more_submit'),
    '#ajax' => array(
      'callback' => 'privatemsg_templates_js',
      'wrapper' => 'templates',
      'method' => 'replace',
      'effect' => 'fade',
    ),
  );

  $form['submit_form'] = array(
    '#type' => 'submit',
    '#weight' => 10,
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Submit handler for admin form.
 */
function privatemsg_templates_admin_settings_submit($form, &$form_state) {
  // We don't want it to submit when we're adding/removing actions.
  if ($form_state['clicked_button']['#id'] == 'edit-submit-form') {
    // If the form's 'block_actions' aren't set, the user has deleted all the
    // rows in the table, so we save an empty array to stop errors in the form
    // builder.
    if (isset($form_state['values']['templates'])) {
      variable_set('privatemsg_templates_actions', _privatemsg_templates_settings_filter($form_state['values']['templates']));
    }
    else {
      variable_set('privatemsg_templates_actions', array());
    }
    drupal_set_message(t('The configuration options have been saved.'));
  }
}

