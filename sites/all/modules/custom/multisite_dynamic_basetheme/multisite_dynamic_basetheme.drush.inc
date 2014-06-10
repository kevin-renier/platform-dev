<?php

/**
* Implementation of hook_drush_command
*/
function multisite_dynamic_basetheme_drush_command() {
  $items = array();
   
 $items['theme-base-theme-set'] = array(
    'description' => "Set a base theme for subtheme.",
    'aliases' => array('tbts'),
     'bootstrap' => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
	  'arguments' => array(
        'base_theme' => 'Machine name of the base theme that you want to set',
       ),
	   'options' => array ('base_theme'=> 'base theme'),
  );
  $items['theme-base-theme-get'] = array(
    'description' => "Get current theme and sub-theme used.",
    'aliases' => array('tbtg'),
     'bootstrap' => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
  );
    $items['theme-base-theme-list'] = array(
    'description' => "List all multisite base theme",
    'aliases' => array('tbtl'),
     'bootstrap' => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
  );
  return $items;
}


/**
* Implementation of drush_hook_
*/
function drush_multisite_dynamic_basetheme_theme_base_theme_list (){
   $options = multisite_dynamic_basetheme_get_base_theme_list ();
   drush_print ("Available base theme :");
   if (!empty ($options)){
   foreach ($options as $machine_name => $option){
    drush_print ("- ".$machine_name,'1');
  }
   }else{
   drush_print ("No base theme available on this site",'1');
   }
}

/**
* Callback function of theme-base-theme-list
*/
function drush_multisite_dynamic_basetheme_theme_base_theme_get() {
  global $theme_info, $base_theme_info;
  drush_print ("Current theme : ".$theme_info->name);
  
  // get the base theme from info file
   $theme_info_file = drupal_parse_info_file (drupal_get_path ('theme',$theme_info->name)."/$theme_info->name.info");
  $base_theme_info_file = (isset ($theme_info_file['base theme'])) ? $theme_info_file['base theme'] : 'not defined';
  drush_print ("Base theme (info file): $base_theme_info_file");
  
  // get the base theme from theme settings
  $settings = theme_get_setting('ec_resp_base_theme_of_subtheme',$theme_key);
  $base_theme_settings = ($settings) ? $settings : 'not defined';
  drush_print ("Base theme (theme settings): ".$base_theme_settings);
  
  // get the current base theme
  $base_theme_current = (isset ($theme_info->info['base theme'])) ? $theme_info->info['base theme'] : 'not defined';
  drush_print (PHP_EOL."Current base theme: ".$base_theme_current);
}

/**
* Callback function of theme-base-theme-set
*/
function drush_multisite_dynamic_basetheme_theme_base_theme_set() {
  // check arguments
  $args = func_get_args();    
  if (count($args) < 1){
     drush_set_error("TBT_MISSING_BASE_THEME","The base theme cannot be set:  missing base_teme argument.");
	exit ();
  }elseif (count($args) > 1){
    drush_set_error("TBT_MISSING_BASE_THEME","The base theme cannot be set: only one argument is expected.");
	exit ();
  }else{
    $base_theme = $args[0];
  }
  
  // check existence of base theme
  $themes = list_themes();
  if (!isset ( $themes[$base_theme])){
     drush_set_error("TBT_BASE_THEME_NOT_FOUND",t("The base theme cannot be set: the theme `@theme` is not found on this site." ,array ('@theme' => $base_theme)));
	 drush_multisite_dynamic_basetheme_theme_base_theme_list();
	 exit ();
  }
  
  // check current theme (is a subtheme)
   global $theme_info;
   if ( ! isset ($theme_info->info['base theme'])){
     drush_set_error("TBT_CURRENT_IS_NOT_A_SUBTHEME",t("The base theme cannot be set: your current theme (@name) is not a sub-theme (`base theme` not found in the info file)." , array ('@name' =>$theme_info->info['name'])));
    exit ();
  }
  
  // get original base theme
  $theme_info_file = drupal_parse_info_file (drupal_get_path ('theme',$theme_info->name)."/$theme_info->name.info");

  // save new base theme
  $var = 'theme_' . $theme_info->name . '_settings';
  $settings = variable_get ($var);
  if ($base_theme === $theme_info_file['base theme']){
    unset ($settings['ec_resp_base_theme_of_subtheme']);
  }else{
    $settings['ec_resp_base_theme_of_subtheme'] = $base_theme;
  }
  
  variable_set ($var,$settings);
	
  // clear cache and rebuild theme data
  _multisite_dynamic_basetheme_rebuild_system_theme ();

  drush_log(t('Base theme of `@name` changed to `@base_theme`', array ('@name' => $theme_info->name, '@base_theme' => $base_theme)), 'ok') ;
}