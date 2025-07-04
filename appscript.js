function doPost(e) {
  // Sheets
  var sheetData = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Data");
  var sheetLastData = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Last_Data");
  var sheetStatus = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Status");

  // Check if header exists, if not add it
  if (sheetData.getLastRow() === 0) {
    sheetData.appendRow(["Timestamp", "Solar Voltage", "Battery Voltage", "Solar Current", "Battery Current", "Solar Power", "Output Power", "PWM Percentage", "Mode", "Load Status", "Efficiency"]);
  }
  
  // Extract data from the POST request
  var solar_voltage = e.parameter.solar_voltage;
  var bat_voltage = e.parameter.bat_voltage;
  var solar_current = e.parameter.solar_current;
  var bat_current = e.parameter.bat_current;
  var solar_power = e.parameter.solar_power;
  var output_power = e.parameter.output_power;
  var pwm_percentage = e.parameter.pwm_percentage;
  var mode_str = e.parameter.mode_str;
  var load_status = e.parameter.load_status;

  // Menghitung Efisiensi
  var efficiency = (solar_power != 0) ? (output_power / solar_power) * 100 : 0;

  // Pastikan efisiensi tidak negatif
  efficiency = Math.max(efficiency, 0);

  // Append the new data to the "Data" sheet
  var newRow = [new Date(), solar_voltage, bat_voltage, solar_current, bat_current, solar_power, output_power, pwm_percentage, mode_str, load_status, efficiency];
  sheetData.appendRow(newRow);

  // Update "Last_Data" sheet with the last 20 entries
  var lastDataRange = sheetData.getRange(Math.max(sheetData.getLastRow() - 19, 2), 1, Math.min(20, sheetData.getLastRow() - 1), sheetData.getLastColumn());
  sheetLastData.clear(); // Clear existing data
  sheetLastData.appendRow(["Timestamp", "Solar Voltage", "Battery Voltage", "Solar Current", "Battery Current", "Solar Power", "Output Power", "PWM Percentage", "Mode", "Load Status", "Efficiency"]);
  lastDataRange.copyTo(sheetLastData.getRange(2, 1));

  // Update "Status" sheet with the most recent entry
  sheetStatus.clear(); // Clear existing data
  sheetStatus.appendRow(["Timestamp", "Solar Voltage", "Battery Voltage", "Solar Current", "Battery Current", "Solar Power", "Output Power", "PWM Percentage", "Mode", "Load Status", "Efficiency"]);
  sheetStatus.appendRow(newRow);

  // Return a response to the ESP32
  return ContentService.createTextOutput("Success");
}

function doGet(e) {
  var action = e.parameter.action;
  
  if (action === 'getStatusData') {
    return ContentService.createTextOutput(JSON.stringify(getStatusData()))
                          .setMimeType(ContentService.MimeType.JSON);
  } else if (action === 'getLastData') {
    return ContentService.createTextOutput(JSON.stringify(getLastData()))
                          .setMimeType(ContentService.MimeType.JSON);
  } else {
    return ContentService.createTextOutput("Invalid action parameter")
                          .setMimeType(ContentService.MimeType.TEXT);
  }
}

// Function to get raw data from "Status" sheet
function getStatusData() {
  var sheetStatus = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Status");
  return sheetStatus.getDataRange().getValues();
}

// Function to get raw data from "Last_Data" sheet
function getLastData() {
  var sheetLastData = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Last_Data");
  return sheetLastData.getDataRange().getValues();
}

