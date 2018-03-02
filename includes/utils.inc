<?php


/**
 * Custom function to get default To address.
 */
function get_to_default($filename){
    $field = variable_get('lib4ri_notif_mail_to_auto_field');
    
    //get file id
    $fid = db_query('select fid from file_managed where filename like :filename and uri like \'%webform%\' order by timestamp desc limit 1', array(':filename' => '%'.$filename.'%'))->fetchField();
    
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
        
    //get submission id
    $res = db_select('webform_submitted_data', 'n')
    ->fields('n', array('sid'))
    ->condition('data', $fid)
    ->execute();
    
    $sub_ids = array();
    foreach ($res as $sid) {
        $sub_ids[] = array(
            'sid' => t($sid->sid),
        );
    }    
    $sid = array_pop($sub_ids)['sid'];

    //get email addresses
    $res = db_select('webform_submitted_data', 'n')
    ->fields('n', array('data'))
    ->condition('sid', $sid)
    ->condition('cid', $cid)
    ->execute();
    
    $emails_a = array();
    foreach ($res as $addr) {
        $emails_a[] = array(
            'addr' => t($addr->data),
        );
    }
    $email = array_pop($emails_a)['addr'];
    
    return $email;
}

/**
 * Custom function to get default Cc address.
 */
function get_cc_default(){
    $field = variable_get('lib4ri_notif_mail_cc_auto_field');
    
    //get file id
    $fid = db_query('select fid from file_managed where filename like :filename and uri like \'%webform%\' order by timestamp desc limit 1', array(':filename' => '%'.$filename.'%'))->fetchField();
    
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
    
    //get submission id
    $res = db_select('webform_submitted_data', 'n')
    ->fields('n', array('sid'))
    ->condition('data', $fid)
    ->execute();
    
    $sub_ids = array();
    foreach ($res as $sid) {
        $sub_ids[] = array(
            'sid' => t($sid->sid),
        );
    }
    $sid = array_pop($sub_ids)['sid'];
    
    //get email addresses
    $res = db_select('webform_submitted_data', 'n')
    ->fields('n', array('data'))
    ->condition('sid', $sid)
    ->condition('cid', $cid)
    ->execute();
    
    $emails_a = array();
    foreach ($res as $addr) {
        $emails_a[] = array(
            'addr' => t($addr->data),
        );
    }
    $email = array_pop($emails_a)['addr'];
    
    return $email;
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
function get_subject_default(AbstractObject $object, $replacement_data){
    $subject = token_replace(variable_get('lib4ri_notif_mail_subject'), $replacement_data);
    return $subject;
}

/**
 * Custom function to get default body.
 */
function get_body_default(AbstractObject $object, $lang){
    if (empty($object)) {return '';}
    global $base_url;
    global $user;
    if ($lang == 'de'){
        $rows = t('Guten Tag'."\n\n".
            'Vielen Dank für die Publikation. Sie ist nun online verfügbar:'."\n".
            url("$base_url/islandora/object/{$object->id}")."\n".
                'Wir bitten Sie/dich die Metadata zu kontrollieren – insbesondere die Verlinkungen von Autoren und Abteilungen – und uns zu informieren wenn Sie/du Fehler finden/findest oder Fragen haben/hast.'."\n\n".
                'Viele Grüsse'."\n\n".
                $user->name
            );
    }
    
    else{
        $rows = t('Good day,'."\n\n".
            'Thank you very much for your publication. “'.$object->label.'” is now available in DORA:'."\n".
            url("$base_url/islandora/object/{$object->id}")."\n".
                'Please take a moment to check the metadata – in particular the linkage to the authors and departments – and let us know if you find any errors or if you have any questions.'."\n\n".
                'Kind regards,'."\n\n".
                $user->name
            );
    }
    return $rows;
}

function get_tokens_data(AbstractObject $object){
    global $user;
    global $base_url;
    $len = variable_get('lib4ri_notif_mail_subj_trunc');
    if (empty($object)) {return '';}
    $short = $object->label;
    $point = 0;
    if (strlen($short)>$len){
        $short .= ' ';
        while ($point<$len){
            $prev = $point;
            $point = strpos($short, ' ', $point+1);
        }
        $short = substr($short, 0, $point).'...';
    }
    
    $data = array('title' => $object->label,
                  'titleshort' => $short,
                  'signature' => $user->name,
                  'objecturl' => url("$base_url/islandora/object/{$object->id}")
    );
    
    return $data;
}