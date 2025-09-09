
#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Structure pour représenter une commande GCode
struct MotionCommand {
  char type; // 'G' ou 'M'
  int code;  // ex. 1 pour G1, 1004 pour M104
  float x, y, z, e, f, s; // Paramètres : X, Y, Z, E, F (vitesse), S (température/vitesse ventilateur)
  bool has_x, has_y, has_z, has_e, has_f, has_s; // Indicateurs de présence
};

// Énumération des codes de commande supportés pour une impression 3D complète
enum class GcodeType {
  // G-Codes
  G0 = 0,
  G1 = 1,
  G2 = 2,
  G3 = 3,
  G20 = 20,
  G21 = 21,
  G28 = 28,
  G29 = 29,
  G90 = 90,
  G91 = 91,
  G92 = 92,
  // M-Codes (décalés pour éviter les conflits)
  M17 = 1017,
  M18 = 1018,
  M82 = 1082,
  M83 = 1083,
  M84 = 1084,
  M104 = 1104,
  M105 = 1105,
  M106 = 1106,
  M107 = 1107,
  M109 = 1109,
  M112 = 1112,
  M114 = 1114,
  M115 = 1115,
  M140 = 1140,
  M190 = 1190,
  M20 = 1020,
  M21 = 1021,
  M22 = 1022,
  M23 = 1023,
  M24 = 1024,
  M25 = 1025,
  M26 = 1026,
  M27 = 1027,
  M28 = 1028,
  M29 = 1029
};

class GcodeParser {
private:
  bool absolute_positioning; // G90 (true) ou G91 (false)
  bool absolute_extrusion;  // M82 (true) ou M83 (false)
  bool parseParameters(String params, MotionCommand &cmd);
  bool parseLinearMovementCommand(String params, MotionCommand &cmd, GcodeType code);
  bool parseArcMovementCommand(String params, MotionCommand &cmd, GcodeType code);
  bool parseSetPositionCommand(String params, MotionCommand &cmd);
  bool parseHomingCommand(String params, MotionCommand &cmd);
  bool parseLevelingCommand(String params, MotionCommand &cmd);
  bool parsePositioningCommand(String params, MotionCommand &cmd, GcodeType code);
  bool parseTemperatureCommand(String params, MotionCommand &cmd, GcodeType code);
  bool parseFanCommand(String params, MotionCommand &cmd, GcodeType code);
  bool parseExtruderCommand(String params, MotionCommand &cmd, GcodeType code);
  bool parseMotorsCommand(String params, MotionCommand &cmd, GcodeType code);
  bool parseSDCommand(String params, MotionCommand &cmd, GcodeType code);
  bool parseReportingCommand(String params, MotionCommand &cmd, GcodeType code);
  bool parseEmergencyCommand(String params, MotionCommand &cmd);

public:
  GcodeParser() : absolute_positioning(true), absolute_extrusion(true) {}
  void init();
  void testParse(String cmd);
  static void parserTask(void *pvParameters);
};

extern GcodeParser gcodeParser;
extern QueueHandle_t gcodeQueue;
extern QueueHandle_t motionQueue;
extern SemaphoreHandle_t errorSemaphore;
