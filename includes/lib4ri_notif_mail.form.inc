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
  $lang = $_GET['lang'];
  if (!empty($pid)){
      $object = islandora_object_load($pid);
      $filename = $_GET['file'];
      $replacements_data = get_tokens_data($object);
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
    '#format' => 'plain_text',
    '#attributes' => array('class' => array('text-full')),
#    '#default_value' => get_body_default($object),
  );
  $form['files'] = array(
    '#title' => t('Attachment'),
    '#type' => 'managed_file',
    '#name' => 'files[]',
    '#attributes' => array('multiple' => 'multiple'),
    '#upload_location' => 'public://sen_mails/',
    '#description' => t("Upload up to 20MB files."),
    '#upload_validators' => array(
      'file_validate_extensions' => array('zip tar tgz taz z gz rar gif png jpg jpeg doc xls ppt sxw sxc sxi sdw sdc sdd pdf'),
      'file_validate_size' => array(20000000),
    ),
 );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send'),
  );
  if (!empty($object)){
      if (variable_get('lib4ri_notif_mail_auto_field_flag')){
          $form['to_id']['#default_value'] = get_to_default($object);
          $form['cc']['#default_value'] = get_cc_default($object);
      }
      $form['subject']['#default_value'] = get_subject_default($object, $lang, $replacements_data);
      $form['body']['#default_value'] = get_body_default($object, $lang, $replacements_data);
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
  if ($user->uid) {
//      $from = '<dora@lib4ri.ch> in behalf of '.$user->name.' ('.$user->mail.')';
      $name = field_get_items('user',user_load($user->uid),variable_get('lib4ri_notif_mail_sig_fname'))[0]['value'].' '.field_get_items('user',user_load($user->uid),variable_get('lib4ri_notif_mail_sig_lname'))[0]['value'];
      $from = "\"{$name}\" <{$user->mail}>";
      $sender = '"DORA Lib4RI" <dora@lib4ri.ch>';
      $ret_path = 'dora@lib4ri.ch';
      $reply_to = 'dora@lib4ri.ch';
  }
  else {
    $from = variable_get('site_mail', '');
    $sender = $from;
    $ret_path = $from;
  }

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
      'Sender' => $sender,
      'Return-Path' => $ret_path,
      'Reply-To' => $reply_to,
  );
  $system = drupal_mail_system($module, $key);
  $message = $system->format($message);
  $message['result'] = $system->mail($message);
  save_mail_data($form_state['values']['object_id'], $email, $message);
  drupal_set_message(t('Your e-mail was sent successfully!'));
}

function lib4ri_notif_mail_admin_form() {
    $form['lib4ri_notif_mail_link_en_flag'] = array(
        '#type' => 'checkbox',
        '#default_value' => variable_get('lib4ri_notif_mail_link_en_flag', TRUE),
        '#title' => t('Show English link'),
    );
    $form['lib4ri_notif_mail_link_en'] = array(
        '#type' => 'textfield',
        '#title' => t('English email link text'),
        '#default_value' => variable_get('lib4ri_notif_mail_link_en', 'Send notification (EN)'),
        '#size' => 40,
        '#maxlength' => 40,
        '#description' => t('The text of the block link to the English version of the email'),
        '#required' => TRUE,
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_en_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    $form['lib4ri_notif_mail_link_de_flag'] = array(
        '#type' => 'checkbox',
        '#default_value' => variable_get('lib4ri_notif_mail_link_de_flag', TRUE),
        '#title' => t('Show German link'),
    );
    $form['lib4ri_notif_mail_link_de'] = array(
        '#type' => 'textfield',
        '#title' => t('German email link text'),
        '#default_value' => variable_get('lib4ri_notif_mail_link_de', 'Send notification (DE)'),
        '#size' => 40,
        '#maxlength' => 40,
        '#description' => t('The text of the block link to the German version of the email'),
        '#required' => TRUE,
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_de_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    
    $form['lib4ri_notif_mail_auto_field_flag'] = array(
        '#type' => 'checkbox',
        '#default_value' => variable_get('lib4ri_notif_mail_auto_field_flag', TRUE),
        '#title' => t('Attempt "To" and "Cc" autofill'),
    );  
    $form['lib4ri_notif_mail_to_auto_field'] = array(
        '#type' => 'textfield',
        '#title' => t('Webform field for "To" autofill'),
        '#default_value' => variable_get('lib4ri_notif_mail_to_auto_field', 'email'),
        '#description' => t('The id of the webform field to be used to look for email addresses'),
        '#required' => TRUE,
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_auto_field_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    $form['lib4ri_notif_mail_cc_auto_field'] = array(
        '#type' => 'textfield',
        '#title' => t('Webform field for "Cc" autofill'),
        '#default_value' => variable_get('lib4ri_notif_mail_cc_auto_field', 'email2'),
        '#description' => t('The id of the webform field to be used to look for email addresses'),
        '#required' => TRUE,
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_auto_field_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    $form['lib4ri_notif_mail_subj_trunc'] = array(
        '#type' => 'textfield',
        '#title' => t('Length for title truncation'),
        '#default_value' => variable_get('lib4ri_notif_mail_subj_trunc', 40),
        '#size' => 2,
        '#maxlength' => 2,
        '#description' => t('The max allowed length for the title provided by the [lib4ri_notif_mail:titleshort] token'),
    );
    $form['lib4ri_notif_mail_subject_en'] = array(
        '#type' => 'textfield',
        '#title' => t('Email subject (EN)'),
        '#default_value' => variable_get('lib4ri_notif_mail_subject_en', 'Your publication "[lib4ri_notif_mail:titleshort]" is now available in DORA'),
        '#description' => t('The Egnlish version of the email subject'),
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_en_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    $form['lib4ri_notif_mail_body_en'] = array(
        '#type' => 'textarea',
        '#title' => t('Email body (EN)'),
        '#default_value' => variable_get('lib4ri_notif_mail_body_en', t(
"Good day,

Thank you very much for your publication. \"[lib4ri_notif_mail:title]\" is now available in DORA:
[lib4ri_notif_mail:objecturl]
Please take a moment to check the metadata – in particular the linkage to the authors and departments – and let us know if you find any errors or if you have any questions

Kind regards,

[lib4ri_notif_mail:signature]")),
        '#description' => t('The English version of the body of the email'),
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_en_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    $form['lib4ri_notif_mail_subject_de'] = array(
        '#type' => 'textfield',
        '#title' => t('Email subject (DE)'),
        '#default_value' => variable_get('lib4ri_notif_mail_subject_de', 'Ihre/deine Publikation "[lib4ri_notif_mail:titleshort]" ist jetzt in DORA verfügbar'),
        '#description' => t('The German version of the email subject'),
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_de_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    
    $form['lib4ri_notif_mail_body_de'] = array(
        '#type' => 'textarea',
        '#title' => t('Email body (DE)'),
        '#default_value' => variable_get('lib4ri_notif_mail_body_de', t(
"Guten Tag
            
Vielen Dank für die Publikation. \"[lib4ri_notif_mail:title]\" ist nun online verfügbar:
[lib4ri_notif_mail:objecturl]
Wir bitten Sie/dich die Metadata zu kontrollieren – insbesondere die Verlinkungen von Autoren und Abteilungen – und uns zu informieren wenn Sie/du Fehler finden/findest oder Fragen haben/hast.
            
Viele Grüsse
            
[lib4ri_notif_mail:signature]")),
        '#description' => t('The German version of the body of the email'),
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_de_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    
    $form['lib4ri_notif_mail_sig_fname'] = array(
        '#type' => 'textfield',
        '#title' => t('First name field'),
        '#default_value' => variable_get('lib4ri_notif_mail_sig_fname', 'field_first_name'),
        '#description' => t('The user\'s field associated with the the first name'),
    );
    $form['lib4ri_notif_mail_sig_lname'] = array(
        '#type' => 'textfield',
        '#title' => t('Family name field'),
        '#default_value' => variable_get('lib4ri_notif_mail_sig_lname', 'field_family_name'),
        '#description' => t('The user\'s field associated with the the family name'),
    );
    
    $form['lib4ri_notif_mail_webform_flag'] = array(
        '#type' => 'checkbox',
        '#default_value' => variable_get('lib4ri_notif_mail_webform_flag', TRUE),
        '#title' => t('Display link to user\'s submission'),
    );
    $form['lib4ri_notif_mail_webform_text'] = array(
        '#type' => 'textfield',
        '#title' => t('Text to be displayed on the link'),
        '#default_value' => variable_get('lib4ri_notif_mail_webform_text', 'View user\'s submission'),
        '#description' => t(''),
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_webform_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    
    $form['lib4ri_notif_mail_webform_htme2txt'] = array(
        '#type' => 'radios',
        '#title' => t('Title\'s HTML tags processing'),
        '#options' => array(
            'remove' => t('Remove'),
            'replace' => t('Replace with plain text emphasis tags')
        ),
        '#default_value' => variable_get('lib4ri_notif_mail_webform_htme2txt', 'remove'),
    );

    $form['lib4ri_notif_mail_webform_bibliography'] = array(
        '#type' => 'textfield',
        '#title' => t('Bibliography webform field'),
        '#default_value' => variable_get('lib4ri_notif_mail_webform_bibliography', 'bibliography'),
        '#description' => t('The id of the bibliography webform field'),
        '#required' => TRUE,
    );
    
    return system_settings_form($form);
}
