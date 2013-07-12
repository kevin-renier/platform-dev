<?php

/**
 *
 * @return type
 */

function multisite_notifications_core_general_settings() {

//Subscription
  $form['subscription_block_settings'] = array(
      '#title' => t('Subscribe block settings'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
  );

  $form['subscription_block_settings']['subscribe_confirmation'] = array(
      '#title' => t('Subscribe confirmation mail'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
  );

  //Subject
  foreach ($languages = language_list() as $language) {
    $form['subscription_block_settings']['subscribe_confirmation']['subscribe_subject_' . $language->language] = array(
        '#title' => t('Subscribe mail subject: ' . $language->name),
        '#type' => 'textfield',
        '#default_value' => variable_get('subscribe_subject_' . $language->language, ''),
    );
  }

  //Body
  foreach ($languages = language_list() as $language) {
    $form['subscription_block_settings']['subscribe_confirmation']['subscribe_message_' . $language->language] = array(
        '#title' => t('Subscribe mail body: ' . $language->name),
        '#type' => 'textarea',
        '#default_value' => variable_get('subscribe_message_' . $language->language, ''),
    );
  }


  //Unsubscription

   $form['subscription_block_settings']['unsubscribe_confirmation'] = array(
      '#title' => t('Unsubscribe confirmation mail'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
  );


    //Subject
  foreach ($languages = language_list() as $language) {
    $form['subscription_block_settings']['unsubscribe_confirmation']['unsubscribe_subject_' . $language->language] = array(
        '#title' => t('Unsubscribe mail subject: ' . $language->name),
        '#type' => 'textfield',
        '#default_value' => variable_get('unsubscribe_subject_' . $language->language, ''),
    );
  }

   //Body
  foreach ($languages = language_list() as $language) {
    $form['subscription_block_settings']['unsubscribe_confirmation']['unsubscribe_message_' . $language->language] = array(
        '#title' => t('Unsubscribe mail body: ' . $language->name),
        '#type' => 'textarea',
        '#default_value' => variable_get('unsubscribe_message_' . $language->language, ''),
    );
  }


  return system_settings_form($form);

}

/**
 *
 * @return \stdClass
 */
function _multisite_notifications_core_mail_template() {
  $template = new stdClass();
  //
  $template->subject='[[site:name]] [subs:type] subscription for {{[current-user:roles:last] == subscriber?subscriber:[current-user:name]}}: [node:title]';
  $template->body = '{{MAIL@| Greetings, {{[current-user:roles:last] == subscriber?subscriber:[current-user:name]}}.
|
| Your subscription on [site:name]
| notifies you of {{[subs:is-new]==0?:a new post:
}}{{[subs:is-updated]#an updated post:
}}{{[subs:is-old]#new comments on:
}}|
}}{{![subs:is-published]#| ***** This post is unpublished! *****
}}{{![subs:category:tid]?:{{[subs:category:tid]==[subs:forum:tid]?:| Category: [subs:category:name]
}}}}{{![subs:forum:tid]?:| Forum: [subs:forum:name]
}}{{![node:author:name]?:| Author: [node:author:name]
}}| Title: [node:title]
{{![subs:terms:field_tags:count]?:| [subs:terms:field_tags:first:vocabulary:name]: {{[subs:terms:field_tags:count]#{{!!#0#, }}[subs:terms:field_tags:index:#0:name]}}
}}{{![subs:is-old]?&nbsp;
[node:body]:{{![subs:has-distinct-summary]?&nbsp;
[node:body]:| Summary:
&nbsp;
[node:summary]}}}}
| LINK: [node:url]
{{![subs:is-old]#{{!![subs:files:field_files:count]#| Attached files:
{{[subs:files:field_files:count]#| [subs:files:field_files:index:#0:url]
}}}}}}{{![subs:comments:count]?| ---------------------------------------------------
:|
| Comments: [subs:comments:count]
| ---------------------------------------------------
[subs:comments:join:
]
}}| Direct unsubscribe link ([subs:type]):
| [subs:unsubscribe-url]
{{MAIL@| This is an automated message. Please do NOT reply to the sender address!
| To manage your subscriptions go to
| [subs:manage-url]
}}';
  $template->comment='| {{[subs:is-new]?New:Updated}} {{[subs:is-published]?:UNPUBLISHED }}comment:
| Author: [comment:author:name]
| Title: [comment:title]
&nbsp;
[comment:body]
| LINK: [comment:url]
{{!![subs:files:field_files:count]#| Attached files:
{{[subs:files:field_files:count]#| [subs:files:field_files:index:#0:url]
}}}}| ---------------------------------------------------';

  return $template;
}