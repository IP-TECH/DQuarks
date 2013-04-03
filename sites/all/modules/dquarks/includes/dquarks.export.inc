<?php

/**
 * @file
 * Provides several different handlers for exporting dquarks results.
 */

/**
 * Implements hook_DquarksExporters().
 *
 * Defines the exporters this module implements.
 *
 * @return array
 *   An "array of arrays", keyed by content-types. The 'handler' slot
 *   should point to the PHP class implementing this flag.
 */
function dquarks_DquarksExporters() {
  return array(
    'excel' => array(
      'title' => t('Microsoft Excel'),
      'description' => t('A file readable by Microsoft Excel.'),
      'handler' => 'DquarksExporterExcel',
    ),
  );
}

/**
 * Return a list of exporters suitable for display in a select list.
 */
function dquarks_export_list() {
  $exporters = dquarks_export_fetch_definition();
  $list = array();
  foreach ($exporters as $name => $exporter) {
    $list[$name] = $exporter['title'];
  }
  return $list;
}

/**
 * Returns a dquarks exporter definition.
 */
function dquarks_export_fetch_definition($format = NULL) {
  static $cache;
  if (!isset($cache)) {
    $cache = module_invoke_all('DquarksExporters');
  }

  if (isset($format)) {
    if (isset($cache[$format])) {
      return $cache[$format];
    }
  }
  else {
    return $cache;
  }
}

/**
 * Instantiates a new dquarks handler based on the format.
 */
function dquarks_export_create_handler($format, $options) {
  $definition = dquarks_export_fetch_definition($format);
  if (isset($definition) && class_exists($definition['handler'])) {
    $handler = new $definition['handler']($options);
  }
  else {
    $handler = new DquarksExporter_broken($options);
  }

  return $handler;
}

class DquarksExporter {

  /**
   * Define the function
   */
  function addRow(&$file_handle, $data) {

  }

  /**
   * Define the function
   */
  function setHeaders($filename) {
    drupal_add_http_header('Content-Type', 'application/force-download');
    drupal_add_http_header('Pragma', 'public');
    drupal_add_http_header('Cache-Control', 'max-age=0');
  }

  /**
   * Define the function
   */
  function bof(&$file_handle) {

  }

  /**
   * Define the function
   */
  function eof(&$file_handle) {

  }

  /**
   *  Define the function
   */
  function setContent($file_name, $node, $format, $options) {

  }

}

class DquarksExporterDelimited extends DquarksExporter {

  /**
   * Call to the constructor function
   */
  function __construct($options) {
    $this->delimiter = isset($options['delimiter']) ? $options['delimiter'] : ',';
    // Convert tabs.
    if ($this->delimiter == '\t') {
      $this->delimiter = "\t";
    }
  }

  /**
   * Add row in the excel file
   */
  function addRow(&$file_handle, $data) {
    foreach ($data as $key => $value) {
      // Escape inner quotes and wrap all contents in new quotes.
      $data[$key] = '"' . str_replace('"', '""', $data[$key]) . '"';

      // Remove <script> tags, which mysteriously cause Excel not to import.
      $data[$key] = preg_replace('!<(/?script.*?)>!', '[$1]', $data[$key]);
    }
    $row = implode($this->delimiter, $data) . "\n";

    if (function_exists('mb_convert_encoding')) {
      $row = mb_convert_encoding($row, 'UTF-16LE', 'UTF-8');
    }

    @fwrite($file_handle, $row);
  }

  /**
   * Set header to the excel file
   */
  function setHeaders($filename) {
    parent::setHeaders($filename);

    // Convert tabs.
    if ($this->delimiter == "\t") {
      $extension = 'tsv';
      $content_type = 'text/tab-separated-values';
    }
    else {
      $extension = 'csv';
      $content_type = 'text/csv';
    }

    drupal_add_http_header('Content-Type', $content_type);
    drupal_add_http_header('Content-Disposition', "attachment; filename=$filename.$extension");
  }

}

/**
 * The Excel exporter currently is just a tab-delimited export.
 */
class DquarksExporterExcel extends DquarksExporterDelimited {

  /**
   * Call to the constructeur function
   */
  function __construct($options) {
    $options['delimiter'] = '\t';
    parent::__construct($options);
  }

  /**
   * Set header to the excel file
   *
   * @param string $filename
   *   The file name
   */
  function setHeaders($filename) {
    drupal_add_http_header('Content-Type', 'application/x-msexcel');
    drupal_add_http_header('Content-Disposition', "attachment; filename=$filename.xls");
    drupal_add_http_header('Pragma', 'public');
    drupal_add_http_header('Cache-Control', 'max-age=0');
  }

  /**
   * Set content in the excel file
   *
   * @param string $file_name
   *   The name of the file.
   * @param object $node
   *   The node object.
   * @param string $format
   *   The format to export data: excel
   * @param array $options
   *   Recover format to export data
   */
  function setContent($file_name, $node, $format, $options) {
    // Create new PHPExcel object.
    module_load_include('module', 'libraries', 'libraries');
    include libraries_get_path('PHPExcel') . '/PHPExcel.php';
    include libraries_get_path('PHPExcel') . '/PHPExcel/Writer/Excel5.php';
    $Fichier_excel = new PHPExcel();
    $Fichier_excel->getProperties()->setCreator(variable_get('site_name', 'Drupal'));
    $Fichier_excel->getProperties()->setTitle($node->title);
    $Fichier_excel->getProperties()->setSubject(t('Submission Details'));
    $Fichier_excel->getProperties()->setDescription("");

    // Add Standard headers information.
    $Fichier_excel->setActiveSheetIndex(0);
    $Fichier_excel->getActiveSheet()->setTitle($node->title);
    $this->addExcelFormatedCell($Fichier_excel, 0, 1, $node->title, 6);
    $this->addExcelFormatedCell($Fichier_excel, 0, 2, t('Submission Details'), 4);
    $this->addExcelFormatedCell($Fichier_excel, 0, 3, t('Time'));
    $this->addExcelFormatedCell($Fichier_excel, 1, 3, t('IP Address'));
    $this->addExcelFormatedCell($Fichier_excel, 2, 3, t('UID'));
    $this->addExcelFormatedCell($Fichier_excel, 3, 3, t('Username'));
    $this->addExcelFormatedCell($Fichier_excel, 4, 3, t('Score'));

    // Add Component headers information.
    $headers = array(array(), array(), array());
    // Load all components.
    dquarks_load_components();
    foreach ($node->dquarks['components'] as $cid => $component) {
      $xlsx_header_function = '_dquarks_xlsx_headers_' . $component['type'];
      if (function_exists($xlsx_header_function)) {
        // Let each component determine its headers.
        $component_header = $xlsx_header_function($component);
        $headers[0] = array_merge($headers[0], (array) $component_header[0]);
        $headers[1] = array_merge($headers[1], (array) $component_header[1]);
        $headers[2] = array_merge($headers[2], (array) $component_header[2]);
      }
    }
    foreach ($headers as $key1 => $header) {
      $mycols_counter = 5;
      foreach ($header as $key2 => $cell) {
        $mycols_counter = $this->addExcelFormatedCell($Fichier_excel, $mycols_counter, $key1 + 1, $cell['value'], $cell['colspan']);
      }
    }
    // Get all the submissions for the node.
    $submissions = dquarks_get_submissions($node->nid);

    // Generate a row for each submission.
    $row_count = 3;
    foreach ($submissions as $sid => $submission) {
      $row = array();
      ++$row_count;
      $row[] = array(
        'colspan' => 0,
        'value' => format_date($submission->submitted, 'small'
        ),
      );
      $row[] = array(
        'colspan' => 0,
        'value' => $submission->remote_addr,
      );
      $row[] = array(
        'colspan' => 0,
        'value' => $submission->uid,
      );
      $row[] = array(
        'colspan' => 0,
        'value' => $submission->name,
      );
      $row[] = array(
        'colspan' => 0,
        'value' => $submission->score . '/' . $submission->max_score,
      );

      foreach ($node->dquarks['components'] as $cid => $component) {
        $xlsx_data_function = '_dquarks_xlsx_data_' . $component['type'];
        if (function_exists($xlsx_data_function)) {
          // Let each component add its data.
          $raw_data = isset($submission->data[$cid]) ? $submission->data[$cid] : NULL;
          $data = $xlsx_data_function($raw_data, $component);
          // Merge data to row.
          $row = array_merge($row, array_values($data));
        }
      }
      // Add data to excel object and merge cell.
      $mycols_counter = 0;
      foreach ($row as $key => $cell) {
        $Fichier_excel->getActiveSheet()->setCellValueByColumnAndRow($mycols_counter, $row_count, $cell['value']);
        if ($cell['colspan'] > 0) {
          $Fichier_excel->getActiveSheet()->mergeCellsByColumnAndRow($mycols_counter, $row_count, $mycols_counter + $cell['colspan'], $row_count);
          $mycols_counter += $cell['colspan'];
        }
        else {
          $mycols_counter++;
        }
      }
    }
    // Write to File.
    $fichier_excel_writer = new PHPExcel_Writer_Excel5($Fichier_excel);
    $fichier_excel_writer->save($file_name);
  }

  /**
   * Add data in defined position of row and column
   *
   * @param PHPExcel $fichier
   *   The file name
   * @param int $col
   *   The comumn id in the excel file
   * @param int $row
   *   The row id in the excel file
   * @param string $value
   *   The data to insert
   * @param int $colspan
   *   The number to add to the column id
   *
   * @return int
   *   The column id
   */
  function addExcelFormatedCell(PHPExcel &$fichier, $col, $row, $value, $colspan = 0) {
    $fichier->getActiveSheet()->setCellValueByColumnAndRow($col, $row, $value);
    if ($colspan > 0) {
      $fichier->getActiveSheet()->mergeCellsByColumnAndRow($col, $row, $col + $colspan, $row);
    }
    $fichier->getActiveSheet()->getColumnDimensionByColumn($col)->setAutoSize(TRUE);
    $fichier->getActiveSheet()->getStyleByColumnAndRow($col, $row)->getAlignment()->setHorizontal(PHPExcel_Style_Alignment::HORIZONTAL_CENTER);
    if ($colspan > 0) {
      return $col + $colspan;
    }
    return $col + 1;
  }

}
