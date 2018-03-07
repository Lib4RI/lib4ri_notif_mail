<?php

function get_submission_id($object){
    //get submission id
    $res = db_select('lib4ri_notif_mail_lib4ridora_webform', 'n')
    ->fields('n', array('sid', 'nid'))
    ->condition('object_id', $object->id)
    ->execute();
    
    $sub_ids = array();
    foreach ($res as $ids) {
        $sub_ids[] = array(
            'sid' => t($ids->sid),
            'nid' => t($ids->nid),
        );
    }
    return(array_pop($sub_ids));
}

function _get_addr_default($object, $field_var){

    $sids = get_submission_id($object);
    
    if (!empty($sids)){
        $field = variable_get($field_var);
        
        //get cid of webform mail components
        $res = db_select('webform_component', 'n')
        ->fields('n', array('cid'))
        ->condition('form_key', $field)
        ->execute();
        
        $email_ids = array();
        foreach ($res as $cid) {
            $email_ids[] = array(
                'cid' => t($cid->cid),
            );
        }
        $cid = array_pop($email_ids)['cid'];
        
        //get email addresses
        $res = db_select('webform_submitted_data', 'n')
        ->fields('n', array('data'))
        ->condition('sid', $sids['sid'])
        ->condition('nid', $sids['nid'])
        ->condition('cid', $cid)
        ->execute();
        
        $emails_a = array();
        foreach ($res as $addr) {
            $emails_a[] = array(
                'addr' => t($addr->data),
            );
        }
        $email = array_pop($emails_a)['addr'];
    }
    else $email = NULL;
    
    return $email;
}

/**
 * Custom function to get default To address.
 */
function get_to_default($object){
    return _get_addr_default($object, 'lib4ri_notif_mail_to_auto_field');
}

/**
 * Custom function to get default Cc address.
 */
function get_cc_default($object){
    return _get_addr_default($object, 'lib4ri_notif_mail_cc_auto_field');
}

/**
 * Custom function to get default Bcc address.
 */
function get_bcc_default(){
    return t('dora@lib4ri.ch');
}

/**
 * Custom function to get default Subject.
 */
function get_subject_default(AbstractObject $object, $lang, $replacement_data){
    if ($lang == 'de'){$subject = token_replace(variable_get('lib4ri_notif_mail_subject_de'), $replacement_data);}
    else{$subject = token_replace(variable_get('lib4ri_notif_mail_subject_en'), $replacement_data);}
    
    return $subject;
}

/**
 * Custom function to get default body.
 */
function get_body_default(AbstractObject $object, $lang, $replacement_data){
    if (empty($object)) {return '';}
    
    if ($lang == 'de'){$rows = token_replace(variable_get('lib4ri_notif_mail_body_de'), $replacement_data);}
    else{$rows = token_replace(variable_get('lib4ri_notif_mail_body_en'), $replacement_data);}
    
    return $rows;
}

function get_tokens_data(AbstractObject $object){
    global $user;
    global $base_url;
    $len = variable_get('lib4ri_notif_mail_subj_trunc');
    if (empty($object)) {return '';}
    $long = trim(str_replace("\n", ' ',drupal_html_to_text($object->label)));
    $short = $long;
    $point = 0;
    if (strlen($short)>$len){
        $short .= ' ';
        while ($point<$len){
            $prev = $point;
            $point = strpos($short, ' ', $point+1);
        }
        $short = substr($short, 0, $point).'...';
    }
    
    $data = array('title' => $long,
                  'titleshort' => $short,
                  'signature' => field_get_items('user',user_load($user->uid),variable_get('lib4ri_notif_mail_sig_fname'))[0]['value'].' '.field_get_items('user',user_load($user->uid),variable_get('lib4ri_notif_mail_sig_lname'))[0]['value'],
                  'objecturl' => url("$base_url/islandora/object/{$object->id}")
    );
    
    return $data;
}