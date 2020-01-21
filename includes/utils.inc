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

function get_object_id($nid, $sid){
    //get submission id
    $res = db_select('lib4ri_notif_mail_lib4ridora_webform', 'n')
    ->fields('n', array('object_id'))
    ->condition('nid', $nid)
    ->condition('sid', $sid)
    ->execute();
    
    $sub_ids = array();
    foreach ($res as $ids) {
        $sub_ids[] = array(
            'obj_id' => t($ids->object_id),
        );
    }
    return($sub_ids);
}

function _get_addr_default_datastream($object, $field_var){
     if (module_exists('lib4ri_staged_ingest')) {
         module_load_include('module', 'lib4ri_staged_ingest', 'lib4ri_staged_ingest');
         if ($field_var == 'submitter')
            return lib4ri_staged_ingest_get_submitter_email($object, $name, $value, $langcode);
         if ($field_var == 'extra')
            return lib4ri_staged_ingest_get_extra_email($object, $name, $value, $langcode);
     }
    
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

function _get_addr_default_solr($object){
    
    $params['f'][0] = 'PID:'.islandora_solr_lesser_escape($object->id);
    
    //$params['q'] = str_replace($base_url.'/', '', $current_url);
    $sq = new IslandoraSolrQueryProcessor();
    
    // Build and execute Apache Solr query.
    $sq->buildQuery('*:*', $params);
    $sq->solrParams['facet'] = "false";
    $sq->solrParams['fl'] = 'mods_name_personal_author_alternativeName_nameIdentifier_authorId_ms';
    $sq->solrLimit = '100000';
    $sq->executeQuery(FALSE, TRUE);
    
    foreach ($sq->islandoraSolrResult['response']['objects'] as $obj) {
        $authors = $obj['solr_doc']['mods_name_personal_author_alternativeName_nameIdentifier_authorId_ms'];
    }
    
    $emails = array();
    foreach ($authors as $author){
//        $params['f'][0] = 'PID:'.islandora_solr_lesser_escape($author);
        $sq = new IslandoraSolrQueryProcessor();
        $sq->buildQuery('*:*', $params);
        $sq->solrParams['fq'] = 'PID:'.islandora_solr_lesser_escape($author);
        $sq->solrParams['facet'] = "false";
        $sq->solrParams['fl'] = 'MADS_email_mt';
        $sq->solrLimit = '10';
        $sq->executeQuery(FALSE, TRUE);
        
        foreach ($sq->islandoraSolrResult['response']['objects'] as $obj) {
            $emails = array_merge($emails, $obj['solr_doc']['MADS_email_mt']);
        }
    }
    
    return implode(',', $emails);
}

/**
 * Custom function to get default To address.
 */
function get_to_default($object){
    $a = _get_addr_default($object, 'lib4ri_notif_mail_to_auto_field');
    if (!empty($a)){
        return $a;
    }
    
    $b = _get_addr_default_datastream($object, 'submitter');
    if (!empty($b)){
        return $b;
    }
    
    $c = _get_addr_default_solr($object);
    if (!empty($c)){
        return $c;
    }
    
}

/**
 * Custom function to get default Cc address.
 */
function get_cc_default($object){
    $a = _get_addr_default($object, 'lib4ri_notif_mail_cc_auto_field');
    if (!empty($a))
        return $a;
    else
        return _get_addr_default_datastream($object, 'extra');
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

function get_tokens_data(AbstractObject $object, &$form_state){
    global $user;
    global $base_url;
    $len = variable_get('lib4ri_notif_mail_subj_trunc');
    if (empty($object)) {return '';}
    if (variable_get('lib4ri_notif_mail_webform_htme2txt') == 'remove')
        $long = trim(strip_tags(html_entity_decode($object->label)));
    else
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
    
    $sids = get_submission_id($object);
    $reminder = null;
    if (empty($sids)){
        $reminder = variable_get('lib4ri_notif_mail_sub_reminder');
    }
    
    $data = array('title' => $long,
                  'titleshort' => $short,
                  'signature' => field_get_items('user',user_load($user->uid),variable_get('lib4ri_notif_mail_sig_fname'))[0]['value'].' '.field_get_items('user',user_load($user->uid),variable_get('lib4ri_notif_mail_sig_lname'))[0]['value'],
                  'objecturl' => url("$base_url/islandora/object/{$object->id}"),
                  'submission_reminder' => $reminder,
                  'openaccesstext' => get_open_access_text($form_state),
                  'workflowtext' => get_workflow_text($form_state),
    );
    
    return $data;
}

function is_notified($id){
    $query = db_select('lib4ri_notif_mail_sent', 'n')
    ->fields('n', array('tstmp', 'mailto', 'cc', 'bcc', subject))
    ->condition('pubid', $id)
    ->orderBy('tstmp', 'DESC') //Most recent first.
    ->execute();
    
    foreach ($query as $send) {
        $items[] = array(
            'tstmp' => t($send->tstmp),
            'to' => t($send->mailto),
            'cc' => t($send->cc),
            'bcc' => t($send->bcc),
            'subject' => t($send->subject),
        );
    }
    
    return !empty($items);
}


function get_open_access_text(&$form_state){
    switch ($form_state['values']['lib4ri_notif_mail_choose_oa']){
        case 'oa':
            return variable_get('lib4ri_notif_mail_open_text');
            break;
            
        case 'goa':
            return variable_get('lib4ri_notif_mail_open2_text');
            break;
            
        case 'ca':
            return variable_get('lib4ri_notif_mail_closed_text');
            break;
    }
    
    return '';
}

function get_workflow_text(&$form_state){
    switch ($form_state['values']['lib4ri_notif_mail_choose_ingest']){
        case 'int':
            return variable_get('lib4ri_notif_mail_alert_ingested_text');
            break;
            
        case 'ext':
            return variable_get('lib4ri_notif_mail_submitted_text');
            break;
        }
    
    return '';
}

