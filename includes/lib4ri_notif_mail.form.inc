<?php

/**
 * @file
 * Mail compose forms.
 */

/**
 * Implements hook_form().
 */
function lib4ri_notif_mail_form() {
    
  global $user;
  module_load_include('inc', 'lib4ri_notif_mail', 'includes/utils');
  $pid = $_GET['id'];
  if (!empty($pid)){
      $object = islandora_object_load($pid);
      $filename = $_GET['file'];
  }
  else{
      $object=NULL;
      $filename = NULL;
  }
//  $mods = islandora_bibliography_get_mods($object->id);
//  $path = drupal_get_path('module', 'lib4ridora');
#  $style = citeproc_default_style();
//   $style = array(
//       'name' => t('LIB4Ri Authors'),
//       'data' => file_get_contents("$path/xml/csl/lib4ri_author.csl"),
//   );
  #$authors = citeproc_bibliography_from_mods($style, $object['MODS']->content);
  #$authors2 = islandora_bibilography_get_title_authors($pid);
  
  $form = array('#attributes' => array('enctype' => 'multipart/form-data'));
  // Exclude anonymous user for rolebased.
  $user_roles = array_diff(user_roles(), array('anonymous user'));
  $form['object_id'] = array(
      '#type' => 'hidden',
      #    '#autocomplete_path' => 'send-mails/mailids',
      '#size' => 20,
      '#title' => t('obid'),
      '#required' => TRUE,
      '#value' => $pid,
  );
  $form['to_id'] = array(
    '#type' => 'textfield',
#    '#autocomplete_path' => 'send-mails/mailids',
    '#size' => 60,
    '#maxlength' => 1000,
    '#title' => t('To'),
    '#required' => TRUE,
    '#description' => t("Enter the email addresses separated by comma(,)."),
#    '#default_value' => get_to_default(),
  );

  $form['cc'] = array(
      '#type' => 'textfield',
      '#size' => 60,
      '#maxlength' => 1000,
      '#title' => t('Cc'),
      '#description' => t("Enter/edit the email addresses separated by comma(,)."),
#      '#default_value' => get_cc_default(),
  );

  $form['bcc'] = array(
      '#type' => 'textfield',
      '#size' => 60,
      '#maxlength' => 1000,
      '#title' => t('Bcc'),
      '#description' => t("Enter/edit the email addresses separated by comma(,)."),
      '#default_value' => get_bcc_default(),
  );
  
  $form['subject'] = array(
    '#type' => 'textfield',
    '#size' => 160,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#title' => t('Subject'),
    '#description' => t("Enter/edit the Subject for the message"),
#    '#default_value' => get_subject_default($object),
  );
  $form['body'] = array(
    '#type' => 'text_format',
    '#title' => t('Message'),
#    '#description' => t("Enter the Body content for mail"),
    '#required' => TRUE,
    '#rows' => 10,
    '#cols' => 60,
    '#attributes' => array('class' => array('text-full')),
#    '#default_value' => get_body_default($object),
  );
  $form['files'] = array(
    '#title' => t('Attachment'),
    '#type' => 'managed_file',
    '#name' => 'files[]',
    '#attributes' => array('multiple' => 'multiple'),
    '#upload_location' => 'public://sen_mails/',
    '#description' => t("Upload up to 5MB files."),
    '#upload_validators' => array(
      'file_validate_extensions' => array('zip tar tgz taz z gz rar gif png jpg jpeg doc xls ppt sxw sxc sxi sdw sdc sdd pdf'),
      'file_validate_size' => array(5000000),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send'),
  );
  if (!empty($object)){
      if (!empty($filename)){
          $form['to_id']['#default_value'] = get_to_default($filename);
          $form['cc']['#default_value'] = get_cc_default($filename);
      }
      $form['subject']['#default_value'] = get_subject_default($object);
      $form['body']['#default_value'] = get_body_default($object);
  }
    return $form;
}

/**
 * Implements hook_form_validate().
 */
function lib4ri_notif_mail_form_validate(&$form, &$form_state) {
  // Role based To address.
  if ($form_state['values']['role_based']) {
    $mailid_array = array();
    module_load_include('inc', 'lib4ri_notif_mail', 'includes/lib4ri_notif_mail.query');
    $roles = $form_state['values']['user_roles'];
    $mailids = lib4ri_notif_mail_query('mailid_from_role', $roles);
    foreach ($mailids as $value) {
      $mailid_array[] = $value->mail;
    }
    $form_state['values']['to_id'] = implode(",", $mailid_array);
  }
  $email = explode(',', $form_state['values']['to_id']);
  foreach ($email as $value) {
    if (!filter_var(trim($value), FILTER_VALIDATE_EMAIL)) {
      form_set_error('to_id', t('Invalid "To Address" please enter valid e-mail ids'));
    }
  }
}

/**
 * Implements hook_form_submit().
 */
function lib4ri_notif_mail_form_submit(&$form, &$form_state) {
  global $user;
  module_load_include('inc', 'lib4ri_notif_mail', 'includes/lib4ri_notif_mail.query');
  // From Address from user login.
//   if ($user->uid) {
//     $from = $user->mail;
//   }
//   else {
//     $from = variable_get('site_mail', '');
//   }
  $from = 'dora@lib4ri.ch';
  $module = 'lib4ri_notif_mail';
  $language = language_default();
  $file = file_load($form_state['values']['files']);
  $report = $form_state['values']['body']['value'];
  $send = FALSE;
  $key = 'lib4ri_notif_mail_send';
  $email = $form_state['values']['to_id'];
//   $params = array(
//       'headers' => array(
//           'Cc' => $form_state['values']['cc'],
//           'Bcc' => $form_state['values']['bcc'],
//       )
//   );
  $message = drupal_mail($module, $key, $email, $language, $params, $from, $send);
  $message['from'] = $from;
  $message['subject'] = $form_state['values']['subject'];
  $message['body'] = array($report);
  $message['params']['attachments'][] = array(
    'filepath' => $file->uri,
    'filename' => $file->filename,
    'filemime' => $file->filemime,
    'list' => TRUE,
  );
  $message['headers']=array(
      'Cc' => $form_state['values']['cc'],
      'Bcc' => $form_state['values']['bcc'],
      'From' => $message['from'],
      'Sender' => $message['from'],
      'Return-Path' => $message['from'],
  );
  $system = drupal_mail_system($module, $key);
  $message = $system->format($message);
  $message['result'] = $system->mail($message);
  save_mail_data($form_state['values']['object_id'], $email, $message);
  drupal_set_message(t('Your e-mail was sent successfully!'));
}
