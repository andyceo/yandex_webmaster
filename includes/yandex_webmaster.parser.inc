<?php

/**
 * @file
 * Contains xml parsers to help DrupalYandexWebmaster class parse
 * responses from Yandex.Webmaster service.
 *
 * @author Andrey Andreev aka andyceo, <andyceo@yandex.ru>, http://andyceo.ruware.com/
 */

/**
 * XML parser for Yandex.Webmaster
 */
class DrupalYandexWebmasterXMLParser {
  public $data;
  protected $current_tag;
  protected $current_data;
  protected $depth = 0;

  protected $error_code = XML_ERROR_NONE;
  protected $error_line;
  protected $error_message;

  /**
   * Parse an XML data file.
   */
  function parse($data) {
    $parser = drupal_xml_parser_create($data);
    xml_set_object($parser, $this);
    xml_set_element_handler($parser, 'start', 'end');
    xml_set_character_data_handler($parser, "data");
    xml_parse($parser, $data);

    $result = TRUE;
    $this->error_code = xml_get_error_code($parser);
    if ($this->error_code != XML_ERROR_NONE) {
      $this->error_line = xml_get_current_line_number($parser);
      $this->error_message = xml_error_string($this->error_code);
      $result = FALSE;
    }

    xml_parser_free($parser);

    return $result;
  }

  function start($parser, $name, $attr) {
    $this->current_tag = $name;
    $this->depth++;
  }

  function end($parser, $name) {
    $this->depth--;
  }

  function data($parser, $data) {
    $this->current_data = trim($data);
  }

  function error() {
    return (object)array(
      'code' => $this->error_code,
      'line' => $this->error_line,
      'message' => $this->error_message,
    );
  }
}


/**
 * XML parser for Yandex.Webmaster
 */
class DrupalYandexWebmasterXMLParserServiceDocument extends DrupalYandexWebmasterXMLParser {
  function start($parser, $name, $attr) {
    parent::start($parser, $name, $attr);
    if ($name == 'COLLECTION' && isset($attr['HREF'])) {
      $this->data = $attr['HREF'];
    }
  }
}


/**
 * XML parser for Yandex.Webmaster host list xml data.
 */
class DrupalYandexWebmasterXMLParserHostList extends DrupalYandexWebmasterXMLParser {
  protected $current_host;
  protected $verification_or_crawling;

  function start($parser, $name, $attr) {
    parent::start($parser, $name, $attr);
    $tag = drupal_strtolower($this->current_tag);

    switch ($name) {
      case 'HOSTLIST':
        $this->data = array();
        break;

      case 'HOST':
        $this->current_host = array();
        if (isset($attr['HREF'])) {
          $this->current_host['href'] = $attr['HREF'];
        }
        break;

      case 'VERIFICATION':
      case 'CRAWLING':
        $this->verification_or_crawling = $name;
        $this->current_host[$tag] = array(
          'state' => $attr['STATE'],
        );
        break;
    }
  }

  function end($parser, $name) {
    parent::end($parser, $name);
    switch ($name) {
      case 'HOST':
        $this->data[] = $this->current_host;
        break;
      case 'VERIFICATION':
      case 'CRAWLING':
        $this->verification_or_crawling = NULL;
        break;
    }
  }

  function data($parser, $data) {
    parent::data($parser, $data);
    $tag = drupal_strtolower($this->current_tag);
    switch ($this->current_tag) {
      case 'DETAILS':
        if ($this->verification_or_crawling) {
          $verification_or_crawling = drupal_strtolower($this->verification_or_crawling);
          $this->current_host[$verification_or_crawling][$tag] = $this->current_data;
        }
        break;
      case 'VIRUSED':
        $this->current_host[$tag] = FALSE;
        if (drupal_strtolower($data) != 'false') {
          $this->current_host[$tag] = TRUE;
        }
        break;
      default:
        $this->current_host[$tag] = $data;
        break;
    }
  }
}


/**
 * XML parser for Yandex.Webmaster host resources list xml data.
 */
class DrupalYandexWebmasterXMLParserHostResources extends DrupalYandexWebmasterXMLParser {
  function start($parser, $name, $attr) {
    parent::start($parser, $name, $attr);
    switch ($name) {
      case 'HOSTLIST':
        $this->data = array();
        break;
      case 'LINK':
        if (isset($attr['HREF'])) {
          $this->data[] = $attr['HREF'];
        }
    }
  }
}

/**
 * XML parser for Yandex.Webmaster host statistics xml data.
 */
class DrupalYandexWebmasterXMLParserHostStatistics extends DrupalYandexWebmasterXMLParserHostList {
  function end($parser, $name) {
    parent::end($parser, $name);
    switch ($name) {
      case 'HOST':
        $this->data = $this->current_host;
        break;
    }
  }
}

/**
 * XML parser for Yandex.Webmaster host verified status xml data.
 */
class DrupalYandexWebmasterXMLParserHostVerified extends DrupalYandexWebmasterXMLParser {
  function start($parser, $name, $attr) {
    parent::start($parser, $name, $attr);
    switch ($name) {
      case 'HOST':
        $this->data = array();
        break;
      case 'VERIFICATION':
        if (isset($attr['STATE'])) {
          $this->data['state'] = $attr['STATE'];
        }
        break;
    }
  }
  function data($parser, $data) {
    parent::data($parser, $data);
    $tag = drupal_strtolower($this->current_tag);
    if ($this->current_data) {
      $this->data[$tag] = $this->current_data;
    }
  }
}

/**
 * XML parser for Yandex.Webmaster host excluded pages xml data.
 */
class DrupalYandexWebmasterXMLParserHostExcluded extends DrupalYandexWebmasterXMLParser {
  private $current_code;
  function start($parser, $name, $attr) {
    parent::start($parser, $name, $attr);
    switch ($name) {
      case 'URL-ERRORS':
        $this->data = array();
        if (isset($attr['COUNT'])) {
          $this->data['count'] = $attr['COUNT'];
        }
        break;
      case 'URL-ERRORS-WITH-CODE':
        if (isset($attr['CODE'])) {
          $this->current_code = $attr['CODE'];
          $this->data[$this->current_code] = array();
        }
        break;
    }
  }
  function data($parser, $data) {
    parent::data($parser, $data);
    $tag = drupal_strtolower($this->current_tag);
    if ($this->current_data) {
      $this->data[$this->current_code][$tag] = $this->current_data;
    }
  }
}

/**
 * XML parser for Yandex.Webmaster host indexed pages xml data.
 */
class DrupalYandexWebmasterXMLParserHostIndexed extends DrupalYandexWebmasterXMLParser {
  function start($parser, $name, $attr) {
    parent::start($parser, $name, $attr);
    switch ($name) {
      case 'HOST':
        $this->data = array();
        break;
    }
  }
  function data($parser, $data) {
    parent::data($parser, $data);
    $tag = drupal_strtolower($this->current_tag);
    switch ($tag) {
      case 'index-count':
        $this->data['count'] = $this->current_data;
        break;
      case 'url':
        $this->data['urls'][] = $this->current_data;
        break;
    }
  }
}

/**
 * XML parser for Yandex.Webmaster host top search queries list xml data.
 */
class DrupalYandexWebmasterXMLParserTopQueries extends DrupalYandexWebmasterXMLParser {
  private $shows_or_clicks;
  private $current_query;

  function start($parser, $name, $attr) {
    parent::start($parser, $name, $attr);
    switch ($name) {
      case 'TOP-QUERIES':
        $this->data = array(
          'shows' => array(),
          'clicks' => array(),
        );
        break;
      case 'TOP-SHOWS':
        $this->shows_or_clicks = 'shows';
        break;
      case 'TOP-CLICKS':
        $this->shows_or_clicks = 'clicks';
        break;
      case 'TOP-INFO':
        $this->current_query = array();
    }
  }

  function end($parser, $name) {
    parent::end($parser, $name);
    switch ($name) {
      case 'TOP-INFO':
        $this->data[$this->shows_or_clicks]['queries'][] = $this->current_query;
        break;
    }
  }

  function data($parser, $data) {
    parent::data($parser, $data);
    $tag = drupal_strtolower($this->current_tag);
    switch ($tag) {
      case 'total-shows-count':
        $this->data['shows']['count'] = $this->current_data;
        break;
      case 'top-shows-percent':
        $this->data['shows']['percent'] = $this->current_data;
        break;
      case 'total-clicks-count':
        $this->data['clicks']['count'] = $this->current_data;
        break;
      case 'top-clicks-percent':
        $this->data['clicks']['percent'] = $this->current_data;
        break;
      case 'query':
      case 'count':
      case 'position':
      case 'is-custom':
        $this->current_query[$tag] = $this->current_data;
        break;
    }
  }
}

/**
 * XML parser for Yandex.Webmaster host indexed pages xml data.
 */
class DrupalYandexWebmasterXMLParserExternalLinks extends DrupalYandexWebmasterXMLParserHostIndexed {
  function data($parser, $data) {
    parent::data($parser, $data);
    $tag = drupal_strtolower($this->current_tag);
    switch ($tag) {
      case 'links-count':
        $this->data['count'] = $this->current_data;
        break;
    }
  }
}
