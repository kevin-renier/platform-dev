<?php

/**
 * @file
 * Contains \Drupal\nexteuropa_laco\LanguageCoverageService.
 */

namespace Drupal\nexteuropa_laco;

/**
 * Class LanguageCoverageService.
 *
 * @package Drupal\nexteuropa_laco
 */
class LanguageCoverageService implements LanguageCoverageServiceInterface {

  /**
   * HTTP method the service will be reacting to.
   */
  const HTTP_METHOD = 'HEAD';

  /**
   * Custom HTTP Header name for requesting service.
   */
  const HTTP_HEADER_SERVICE_NAME = 'EC-Requester-Service';

  /**
   * Custom HTTP Header value idendifying the requesting service.
   */
  const HTTP_HEADER_SERVICE_VALUE = 'WEBTOOLS LACO';

  /**
   * Custom HTTP Header name for requested language.
   */
  const HTTP_HEADER_LANGUAGE_NAME = 'EC-LACO-lang';

  /**
   * Contains singleton instance.
   *
   * @var LanguageCoverageService
   */
  private static $instance = NULL;

  /**
   * Response status.
   *
   * @var string
   */
  private $status = '';

  /**
   * {@inheritdoc}
   */
  static public function getInstance() {
    if (!self::$instance) {
      self::$instance = new static();
    }
    return self::$instance;
  }

  /**
   * {@inheritdoc}
   */
  static public function isServiceRequest() {
    $header = self::getHeaderKey(self::HTTP_HEADER_SERVICE_NAME);
    $result = isset($_SERVER['REQUEST_METHOD']) && ($_SERVER['REQUEST_METHOD'] == self::HTTP_METHOD);
    $result = $result && isset($_SERVER[$header]) && ($_SERVER[$header] == self::HTTP_HEADER_SERVICE_VALUE);
    return $result;
  }

  /**
   * {@inheritdoc}
   */
  public function hookBoot() {
    $path = $this->sanitizePath($_GET['q']);
    $language = $this->getRequestedLanguage();

    if (!$language) {
      $this->setStatus('400 Bad request');
    }
    elseif (!$this->isValidLanguage($language)) {
      $this->setStatus('404 Not found');
    }

    // Check for node language coverage.
    if ($this->isNodePath($path)) {
      if ($this->assertNodeLanguageCoverage($path, $language)) {
        $this->setStatus('200 OK');
      }
      else {
        $this->setStatus('404 Not found');
      }
    }

    if ($this->hasStatus()) {
      drupal_exit();
    }
  }

  /**
   * {@inheritdoc}
   */
  public function hookInit() {
    if (!$this->hasStatus()) {
      $path = $this->sanitizePath($_GET['q']);
      $page_callback_result = MENU_NOT_FOUND;

      if ($router_item = menu_get_item($path)) {
        if ($router_item['access']) {
          if ($router_item['include_file']) {
            require_once DRUPAL_ROOT . '/' . $router_item['include_file'];
          }
          $page_callback_result = call_user_func_array($router_item['page_callback'], $router_item['page_arguments']);
        }
        else {
          $page_callback_result = MENU_ACCESS_DENIED;
        }
      }

      switch ($page_callback_result) {
        case MENU_NOT_FOUND:
          $this->setStatus('404 Not found');
          break;

        case MENU_ACCESS_DENIED:
          $this->setStatus('403 Forbidden');
          break;
      }
      drupal_exit();
    }
  }

  /**
   * Check if the given path is a node path.
   *
   * @param string $path
   *    Relative Drupal path.
   *
   * @return bool
   *    TRUE if given path is a node path, FALSE otherwise.
   */
  protected function isNodePath($path) {
    return (bool) preg_match('/node\/\d*/', $path);
  }

  /**
   * Assert node language coverage given its relative path.
   *
   * @param string $path
   *    Relative Drupal path.
   * @param string $language
   *    Language code.
   *
   * @return bool
   *    TRUE if given node is available in the given language, FALSE otherwise.
   */
  protected function assertNodeLanguageCoverage($path, $language) {
    list(, $nid) = explode('/', $path);
    $translations = db_select('entity_translation', 't')
      ->fields('t', ['language'])
      ->condition('t.entity_type', 'node')
      ->condition('t.entity_id', $nid)
      ->condition('t.status', 1)
      ->execute()
      ->fetchAllAssoc('language');
    return $translations && array_key_exists($language, $translations);
  }

  /**
   * Check whereas thew given language is enabled on the current site.
   *
   * @param string $language
   *    Language code.
   *
   * @return bool
   *    TRUE for a valid language, FALSE otherwise.
   */
  protected function isValidLanguage($language) {
    return (bool) db_select('languages', 'l')
      ->fields('l', ['language'])
      ->condition('l.language', $language)
      ->condition('l.enabled', 1)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get list of available languages.
   *
   * @return array
   *    List of available languages
   */
  protected function getAvailableLanguages() {
    $languages = db_select('languages', 'l')
      ->fields('l', ['language'])
      ->condition('l.enabled', 1)
      ->execute()
      ->fetchAllAssoc('language');
    return array_keys($languages);
  }

  /**
   * Set HTTP response status code.
   *
   * @param string $status
   *    Set response status.
   */
  protected function setStatus($status) {
    $this->status = $status;
    $this->setHeader('Status', $status);
  }

  /**
   * Status property getter.
   *
   * @return string
   *    Return current response status.
   */
  protected function getStatus() {
    return $this->status;
  }

  /**
   * Check whereas the status has already been set ot not.
   *
   * @return bool
   *    TRUE status has been set, FALSE otherwise.
   */
  protected function hasStatus() {
    return (bool) $this->status;
  }

  /**
   * Set HTTP response header.
   *
   * @param string $name
   *    Header name.
   * @param string $value
   *    Header value.
   */
  protected function setHeader($name, $value) {
    drupal_add_http_header($name, $value);
  }

  /**
   * Get requested language.
   *
   * @return string|FALSE
   *    The requested language, FALSE if none found.
   */
  protected function getRequestedLanguage() {
    $header = self::getHeaderKey(self::HTTP_HEADER_LANGUAGE_NAME);
    if (isset($_SERVER[$header]) && !empty($_SERVER[$header])) {
      return $_SERVER[$header];
    }
    return FALSE;
  }

  /**
   * Remove language negotiation suffix from the end of the URL, if any.
   *
   * @param string $url
   *    Requested URL.
   *
   * @return string
   *    Sanitized URL.
   */
  protected function removeLanguageNegotiationSuffix($url) {
    $languages = implode('|_', $this->getAvailableLanguages());
    return preg_replace("/(_{$languages})$/i", '', $url);
  }

  /**
   * Get source path given its alias. Return input path if no alias is found.
   *
   * @param string $path
   *    Relative Drupal path.
   *
   * @return string
   *    Source path if any, input path if none.
   */
  protected function getSourcePath($path) {
    $result = db_select('url_alias', 'a')
      ->fields('a', ['source'])
      ->condition('a.alias', $path)
      ->execute()
      ->fetchAssoc();
    if (is_array($result) && !empty($result)) {
      return array_shift($result);
    }
    return $path;
  }

  /**
   * Sanitize path.
   *
   * @param string $path
   *    Relative Drupal path.
   *
   * @return string
   *    Sanitized path.
   */
  protected function sanitizePath($path) {
    $path = $this->removeLanguageNegotiationSuffix($path);
    $path = $this->getSourcePath($path);
    return $path;
  }

  /**
   * Convert HTTP header name into $_SERVER array key.
   *
   * @param string $header
   *    Header name as provided by the HTTP request.
   *
   * @return string
   *    Header name as a $_SERVER array key.
   */
  static protected function getHeaderKey($header) {
    $header = 'HTTP_' . strtoupper($header);
    return str_replace('-', '_', $header);
  }

}
