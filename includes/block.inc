<?php


/**
 * @file
 * Block functionality for lib4ri_notif_mail.
 */

function lib4ri_notif_mail_get_block_links(AbstractObject $object) {
    $datastreams = iterator_to_array($object);
    
    $query = array('id' => $object->id,
        'destination' => current_path(),);
    
    if (variable_get('lib4ri_notif_mail_link_en_flag')){
        $query['lang'] = 'en';
        $out = l(variable_get('lib4ri_notif_mail_link_en'),LIB4RI_NOTIF_MAIL_FORM_LINK, array('query' => $query)).'<br>';
    }
    if (variable_get('lib4ri_notif_mail_link_de_flag')){
        $query['lang'] = 'de';
        $out .= l(variable_get('lib4ri_notif_mail_link_de'),LIB4RI_NOTIF_MAIL_FORM_LINK, array('query' => $query)).'<br>';
    }
                
    $query = db_select('lib4ri_notif_mail_sent', 'n')
    ->fields('n', array('tstmp', 'mailto', 'cc', 'bcc', 'subject'))
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
            $out .= '<br> Sent message: <br>';
        else 
            $out .= '<br> Sent messages: <br>';
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
    
    if(variable_get('lib4ri_notif_mail_webform_flag')){
        module_load_include('inc', 'lib4ri_notif_mail', 'includes/utils');
        $sids = get_submission_id($object);
        if (!empty($sids)){
#        $out .= '<br>'.l(variable_get('lib4ri_notif_mail_webform_text'), variable_get('lib4ri_notif_mail_webform_url').'/'.$sid).'<br>';
         $out .= '<br>'.l(variable_get('lib4ri_notif_mail_webform_text'), 'node/'.$sids['nid'].'/submission/'.$sids['sid']).'<br>';
        }
    }
    
    return $out;
}