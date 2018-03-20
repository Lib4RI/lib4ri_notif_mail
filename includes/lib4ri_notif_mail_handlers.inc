<?php
/**
 * 
 *
 * 
 */
 class lib4ri_notif_mail_handler_field_custom_handler extends views_handler_field {
   function query() {
     // do nothing -- to override the parent query.
   }

   function option_definition() {
     $options = parent::option_definition();

     // Override the alter text option to always alter the text.
     $options['alter']['contains']['alter_text'] = array('default' => TRUE);
     return $options;
   }

   function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

//     Remove the checkbox
     unset($form['alter']['alter_text']);
     unset($form['alter']['text']['#dependency']);
     unset($form['alter']['text']['#process']);
   }

   function render($values) {
     // Render your content.
     module_load_include('inc', 'lib4ri_notif_mail', 'includes/utils');
     $obj_ids = get_object_id($values->webform_submissions_nid, $values->sid);
     $ret = '';
     $color = 'black';
     if (sizeof($obj_ids) > 0){
         $ret = '<font color="red">Ingested ('.sizeof($obj_ids).')';
         if (sizeof($obj_ids) == 1){
             $ret = '<font color="black">Ingested';
         }
         
         if (is_notified($obj_ids[0]['obj_id']))
             $ret .= '  -   User Notified';
         $ret .= '</font>';
     }
          
     return $ret;
   }
}