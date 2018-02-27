<?php


/**
 * @file
 * Block functionality for lib4ri_notif_mail.
 */

function lib4ri_notif_mail_get_block_links(AbstractObject $object) {
    $out = l('Send notification',LIB4RI_NOTIF_MAIL_FORM_LINK, array('query' => array('id' => $object->id, 'destination' => current_path())));
                
    $query = db_select('lib4ri_notif_mail_sent', 'n')
    ->fields('n', array('tstmp', 'mailto', 'cc', 'bcc', subject))
    ->condition('pubid', $object->id) 
    ->orderBy('tstmp', 'DESC') //Most recent first.
    ->execute();
    $items = array();
    // Iterate over the result set and format as links.
    foreach ($query as $send) {
        $items[] = array(
            'tstmp' => t($send->tstmp),
            'to' => t($send->mailto),
            'cc' => t($send->cc),
            'bcc' => t($send->bcc),
            'subject' => t($send->subject),
        );
    }
    
    if (!empty($items)){
        if (count($items)==1)
            $out .= '<br><br> Sent message: <br>';
        else 
            $out .= '<br><br> Sent messages: <br>';
        foreach ($items as $item){
            $out .= '
                <fieldset class="webform-component-fieldset collapsible collapsed form-wrapper">
                    <legend>
                        <span class="fieldset-legend">'.date('d.m.Y H:i',$item['tstmp']).'</span>
                    </legend>
                    <div class="fieldset-wrapper">
                        <div><font size = "2">
                            To: '.$item['to'].'<br>
                            Cc: '.$item['cc'].'<br>
                            Bcc: '.$item['bcc'].'<br>
                            Subject: '.$item['subject'].'
                        </font></div>
                    </div>
                </fieldset>';
        }
    }
    
    return $out;
}