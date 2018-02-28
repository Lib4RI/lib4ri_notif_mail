<?php


/**
 * Custom function to get default To address.
 */
function get_to_default($filename){
    //get file id
    $fid = db_query('select fid from file_managed where filename like :filename and uri like \'%webform%\' order by timestamp desc limit 1', array(':filename' => '%'.$filename.'%'))->fetchField();
    
    //get cid of webform mail components 
    $res = db_select('webform_component', 'n')
    ->fields('n', array('cid'))
    ->condition('form_key', 'email')
    ->execute();
    
    $email_ids = array();
    foreach ($res as $cid) {
        $email_ids[] = array(
            'cid' => t($cid->cid),
        );
    }
    $cid = array_pop($email_ids)['cid'];
    
//     //get cid of webform file components
//     $res = db_select('webform_component', 'n')
//     ->fields('n', array('cid'))
//     ->condition('type', 'file')
//     ->execute();
    
//     $file_ids = array();
//     foreach ($res as $cid) {
//         $file_ids[] = array(
//             'cid' => t($cid->cid),
//         );
//     }
    
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
    return '';
}

/**
 * Custom function to get default Bcc address.
 */
function get_bcc_default(){
    global $user;
    return t($user->mail);
}

/**
 * Custom function to get default Subject.
 */
function get_subject_default(AbstractObject $object){
    if (empty($object)) {return '';}
    return t('Your publication "'.$object->label.'" is available in DORA');
}

/**
 * Custom function to get default body.
 */
function get_body_default(AbstractObject $object){
    if (empty($object)) {return '';}
    global $base_url;
    $rows = t('Your publication "'.$object->label.'" has been ingested in DORA'."\n\n".
              'Please check the correcteness of the data at the following link:'."\n".
               url("$base_url/islandora/object/{$object->id}")
        );
    return $rows;
}

