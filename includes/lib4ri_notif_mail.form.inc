<?php

/**
 * @file
 * Mail compose forms.
 */

/**
 * Implements hook_form().
 */
function lib4ri_notif_mail_form($form, &$form_state) {
    if (isset($form_state['storage']['send'])) {
        return lib4ri_notif_mail_send_form($form, $form_state);
    }

    return lib4ri_notif_mail_choose_content_form($form, $form_state);
}

function lib4ri_notif_mail_choose_content_form($form, &$form_state) {
    module_load_include('inc', 'lib4ri_notif_mail', 'includes/utils');
    
    $form['lib4ri_notif_mail_choose_ingest'] = array(
        '#type' => 'radios',
        '#title' => t('Publication source'),
        '#options' => array(
            'int' => variable_get('lib4ri_notif_mail_alert_ing_txt'),
            'ext' => variable_get('lib4ri_notif_mail_auth_sub_txt')
        ),
//        '#default_value' => variable_get('lib4ri_notif_mail_choose_ingest', 'int'),
        '#default_value' => get_choose_ingest_default(),
    );

    $form['lib4ri_notif_mail_choose_oa'] = array(
        '#type' => 'radios',
        '#title' => t('Open acces'),
        '#options' => array(
            'oa' => variable_get('lib4ri_notif_mail_gold_txt'),
            'goa' => variable_get('lib4ri_notif_mail_green_txt'),
            'ca' => variable_get('lib4ri_notif_mail_nooa_txt')
        ),
        '#default_value' => variable_get('lib4ri_notif_mail_choose_oa', 'oa'),
    );
    
    $form['next'] = array(
        '#type' => 'submit',
        '#value' => t('Next'),
        '#id' => 'edit-send',
    );
    
    return $form;
}

function lib4ri_notif_mail_send_form($form, &$form_state) {
    
  global $user;
  module_load_include('inc', 'lib4ri_notif_mail', 'includes/utils');
  $pid = $_GET['id'];
  $lang = $_GET['lang'];
  if (!empty($pid)){
      $object = islandora_object_load($pid);
      if (isset($_GET['file'])){
        $filename = $_GET['file'];
      }
      else {
          $filename = NULL;
      }
      $replacements_data = get_tokens_data($object, $form_state);
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
    '#id' => 'send',
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

// function lib4ri_notif_mail_form_validate($form, &$form_state) {
    
// }

function lib4ri_notif_mail_choose_content_validate($form, &$form_state) {
    
}

function lib4ri_notif_mail_send_validate($form, &$form_state) {
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
function lib4ri_notif_mail_form_submit($form, &$form_state) {
    
    switch ($form_state['triggering_element']['#id']){
        case 'edit-send':
            $form_state['storage']['send'] = TRUE;
            $form_state['rebuild'] = TRUE;
        break;
    
        
        case 'send':
              global $user;
              module_load_include('inc', 'lib4ri_notif_mail', 'includes/lib4ri_notif_mail.query');
              // From Address from user login.
              if ($user->uid) {
            //      $from = '<dora@lib4ri.ch> in behalf of '.$user->name.' ('.$user->mail.')';
                  $name = mb_encode_mimeheader(field_get_items('user',user_load($user->uid),variable_get('lib4ri_notif_mail_sig_fname'))[0]['value'].' '.field_get_items('user',user_load($user->uid),variable_get('lib4ri_notif_mail_sig_lname'))[0]['value']);
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
              $email = str_replace(';',',',$form_state['values']['to_id']);
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
                  'Cc' => str_replace(';',',',$form_state['values']['cc']),
                  'Bcc' => str_replace(';',',',$form_state['values']['bcc']),
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
              break;
    }
}


function lib4ri_notif_mail_get_step($form, &$form_state) {
    
    $form_state['storage']['stage'] = isset($form_state['storage']['stage']) ? $form_state['storage']['stage'] : 1;
    
    $form_stages = array(
        'choose_content' => 1,
        'send' => 2,
    );
    
    if (isset($form_stages[$form_state['storage']['stage']])) {
        $current_step = $form_stages[$form_state['storage']['stage']];
    }
    else {
        $current_step = 1;
    }
        
    return $form;
    
}


/***************************************************************************************/
function lib4ri_notif_mail_admin_form() {
    global $base_url;
    
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
    
    $form['lib4ri_notif_mail_alert_ingested_text'] = array(
        '#type' => 'textarea',
        '#title' => t('Publication source - Alert ingested (EN)'),
        '#default_value' => variable_get('lib4ri_notif_mail_alert_ingested_text', t()),
        '#description' => t(''),
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_en_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    
    $form['lib4ri_notif_mail_submitted_text'] = array(
        '#type' => 'textarea',
        '#title' => t('Publication source - Author submitted (EN)'),
        '#default_value' => variable_get('lib4ri_notif_mail_submitted_text', t()),
        '#description' => t(''),
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_en_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    
    $form['lib4ri_notif_mail_open_text'] = array(
        '#type' => 'textarea',
        '#title' => t('Gold Open Access (EN)'),
        '#default_value' => variable_get('lib4ri_notif_mail_open_text', t()),
        '#description' => t(''),
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_en_flag"]' => array('checked' => TRUE),
            ),
        ),
    );

    $form['lib4ri_notif_mail_open2_text'] = array(
        '#type' => 'textarea',
        '#title' => t('Green Open Access (EN)'),
        '#default_value' => variable_get('lib4ri_notif_mail_open2_text', t()),
        '#description' => t(''),
        '#states' => array(
            'visible' => array(
                ':input[name="lib4ri_notif_mail_link_en_flag"]' => array('checked' => TRUE),
            ),
        ),
    );
    
    $form['lib4ri_notif_mail_closed_text'] = array(
        '#type' => 'textarea',
        '#title' => t('NO Open Access (EN)'),
        '#default_value' => variable_get('lib4ri_notif_mail_closed_text', t()),
        '#description' => t(''),
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
    
    $form['lib4ri_notif_mail_sub_reminder'] = array(
        '#type' => 'textarea',
        '#size' => 100,
        '#title' => t('Submission form reminder'),
        '#default_value' => variable_get('lib4ri_notif_mail_sub_reminder', 'For further submissions, please use our new submission form '.$base_url.'/submit'),
        '#description' => t('The text to display to remind users to use the submission form'),
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
    
    $form['lib4ri_notif_mail_webform_pub_link_text'] = array(
        '#type' => 'textfield',
        '#title' => t('Text for the link to the Publication'),
        '#default_value' => variable_get('lib4ri_notif_mail_webform_pub_link_text', 'Pub: '),
        '#description' => t('The id of the bibliography webform field'),
        '#required' => TRUE,
    );

    $form['lib4ri_notif_mail_alert_ing_txt'] = array(
        '#type' => 'textfield',
        '#title' => t('Text for alert ingested publications'),
        '#default_value' => variable_get('lib4ri_notif_mail_alert_ing_txt', 'Alert ingested publication'),
        '#description' => t(''),
        '#required' => TRUE,
    );

    $form['lib4ri_notif_mail_auth_sub_txt'] = array(
        '#type' => 'textfield',
        '#title' => t('Text for author submitted publications'),
        '#default_value' => variable_get('lib4ri_notif_mail_auth_sub_txt', 'Author submitted publication'),
        '#description' => t(''),
        '#required' => TRUE,
    );

    $form['lib4ri_notif_mail_gold_txt'] = array(
        '#type' => 'textfield',
        '#title' => t('Text for gold open access publications'),
        '#default_value' => variable_get('lib4ri_notif_mail_gold_txt', 'Gold Open access'),
        '#description' => t(''),
        '#required' => TRUE,
    );

    $form['lib4ri_notif_mail_green_txt'] = array(
        '#type' => 'textfield',
        '#title' => t('Text for green open access publications'),
        '#default_value' => variable_get('lib4ri_notif_mail_green_txt', 'Green Open access'),
        '#description' => t(''),
        '#required' => TRUE,
    );
    
    $form['lib4ri_notif_mail_nooa_txt'] = array(
        '#type' => 'textfield',
        '#title' => t('Text for not open access publications'),
        '#default_value' => variable_get('lib4ri_notif_mail_nooa_txt', 'NOT Open access'),
        '#description' => t(''),
        '#required' => TRUE,
    );
    
    $form['lib4ri_notif_mail_solr_auth_id'] = array(
        '#type' => 'textfield',
        '#title' => t('Solr field for authors ID'),
        '#default_value' => variable_get('lib4ri_notif_mail_solr_auth_id', ''),
        '#description' => t(''),
        '#required' => TRUE,
    );

    $form['lib4ri_notif_mail_solr_auth_fullname'] = array(
        '#type' => 'textfield',
        '#title' => t('Solr field for authors full name'),
        '#default_value' => variable_get('lib4ri_notif_mail_solr_auth_fullname', 'MADS_fullname_mt'),
        '#description' => t(''),
        '#required' => TRUE,
    );

    $form['lib4ri_notif_mail_solr_auth_mail'] = array(
        '#type' => 'textfield',
        '#title' => t('Solr field for authors e-mail'),
        '#default_value' => variable_get('lib4ri_notif_mail_solr_auth_mail', 'MADS_email_mt'),
        '#description' => t(''),
        '#required' => TRUE,
    );
    
    return system_settings_form($form);
}

function lib4ri_notif_mail_comment_form($form, &$form_state) {
    $form['comment'] = array(
        '#type' => 'textarea',
        '#title' => t('Comment'),
        '#required' => FALSE,
    );
    
    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Insert',
    );
    
    return $form;
}

function lib4ri_notif_mail_form_lib4ri_notif_mail_comment_form_alter(&$form, &$form_state) {
    module_load_include('inc', 'lib4ri_notif_mail', 'includes/utils');
    $nid = trim($_GET['nid'],'_');
    $sid = trim($_GET['sid'],'_');
    
    $form['comment']['#default_value'] = get_comment($nid, $sid)[0]['comment'];
    
    return $form;
}

function lib4ri_notif_mail_comment_form_submit($form, &$form_state) {
    
    module_load_include('inc', 'lib4ri_notif_mail', 'includes/utils');
    $nid = trim($_GET['nid'],'_');
    $sid = trim($_GET['sid'],'_');
    
    edit_comment($nid, $sid, $form_state['values']['comment']);
}