<?php

/**
 * @file
 * Database querys and Autocomplete json handlers.
 */

function save_mail_data($pubid, $to, $message){
    $data = array(
        'pubid' => $pubid,
        'tstmp' => REQUEST_TIME,
        'mailto' => $to,
        'cc' => $message['headers']['Cc'],
        'bcc' => $message['headers']['Bcc'],
        'subject' => $message['subject'],
    );
    drupal_write_record('lib4ri_notif_mail_sent', $data);
}
