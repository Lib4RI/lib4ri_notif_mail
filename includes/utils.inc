<?php


/**
 * Custom function to get default To address.
 */
function get_to_default(){
    return '';
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

