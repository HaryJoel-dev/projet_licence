
#include "gcode_parser.h"
#include "../debug_manager.h"
#include "system_manager.h"

GcodeParser gcodeParser;

void GcodeParser::init() {
  DEBUG_PRINTF_AUTO("Initialisation du Gcode Parser");
  absolute_positioning = true; // G90 par défaut
  absolute_extrusion = true;  // M82 par défaut
}

void GcodeParser::testParse(String cmd) {
  cmd.trim();
  if (cmd.isEmpty()) {
    DEBUG_PRINTF_AUTO("Test: Commande vide ignorée");
    Serial.println("ERROR: Empty command");
    return;
  }
  DEBUG_PRINTF_AUTO("Test: Parsing commande '%s'", cmd.c_str());

  MotionCommand parsed_cmd = {0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, false, false, false, false, false};
  int space_pos = cmd.indexOf(' ');
  String params = (space_pos == -1) ? "" : cmd.substring(space_pos + 1);
  String code_str = cmd.substring(0, space_pos == -1 ? cmd.length() : space_pos);

  if (!code_str.startsWith("G") && !code_str.startsWith("M")) {
    DEBUG_PRINTF_AUTO("Test: Type de commande inconnu '%s'", code_str.c_str());
    Serial.println("ERROR: Invalid command type");
    return;
  }

  parsed_cmd.type = code_str.charAt(0);
  int raw_code = code_str.substring(1).toInt();
  parsed_cmd.code = raw_code;
  if (parsed_cmd.type == 'M') {
    parsed_cmd.code += 1000; // Décaler les M codes
  }

  bool valid = false;
  switch (static_cast<GcodeType>(parsed_cmd.code)) {
    case GcodeType::G0:
    case GcodeType::G1:
      valid = parseLinearMovementCommand(params, parsed_cmd, static_cast<GcodeType>(parsed_cmd.code));
      break;
    case GcodeType::G2:
    case GcodeType::G3:
      valid = parseArcMovementCommand(params, parsed_cmd, static_cast<GcodeType>(parsed_cmd.code));
      break;
    case GcodeType::G92:
      valid = parseSetPositionCommand(params, parsed_cmd);
      break;
    case GcodeType::G28:
      valid = parseHomingCommand(params, parsed_cmd);
      break;
    case GcodeType::G29:
      valid = parseLevelingCommand(params, parsed_cmd);
      break;
    case GcodeType::G20:
    case GcodeType::G21:
    case GcodeType::G90:
    case GcodeType::G91:
      valid = parsePositioningCommand(params, parsed_cmd, static_cast<GcodeType>(parsed_cmd.code));
      break;
    case GcodeType::M104:
    case GcodeType::M109:
    case GcodeType::M140:
    case GcodeType::M190:
      valid = parseTemperatureCommand(params, parsed_cmd, static_cast<GcodeType>(parsed_cmd.code));
      break;
    case GcodeType::M106:
    case GcodeType::M107:
      valid = parseFanCommand(params, parsed_cmd, static_cast<GcodeType>(parsed_cmd.code));
      break;
    case GcodeType::M82:
    case GcodeType::M83:
      valid = parseExtruderCommand(params, parsed_cmd, static_cast<GcodeType>(parsed_cmd.code));
      break;
    case GcodeType::M17:
    case GcodeType::M18:
    case GcodeType::M84:
      valid = parseMotorsCommand(params, parsed_cmd, static_cast<GcodeType>(parsed_cmd.code));
      break;
    case GcodeType::M20:
    case GcodeType::M21:
    case GcodeType::M22:
    case GcodeType::M23:
    case GcodeType::M24:
    case GcodeType::M25:
    case GcodeType::M26:
    case GcodeType::M27:
    case GcodeType::M28:
    case GcodeType::M29:
      valid = parseSDCommand(params, parsed_cmd, static_cast<GcodeType>(parsed_cmd.code));
      break;
    case GcodeType::M105:
    case GcodeType::M114:
    case GcodeType::M115:
      valid = parseReportingCommand(params, parsed_cmd, static_cast<GcodeType>(parsed_cmd.code));
      break;
    case GcodeType::M112:
      valid = parseEmergencyCommand(params, parsed_cmd);
      break;
    default:
      DEBUG_PRINTF_AUTO("Test: Code %c%d non supporté", parsed_cmd.type, raw_code);
      Serial.println("ERROR: Unsupported command");
      return;
  }

  if (valid) {
    DEBUG_PRINTF_AUTO("Test: Commande valide, type=%c, code=%d", parsed_cmd.type, raw_code);
    Serial.println("OK: Command parsed");
  } else {
    DEBUG_PRINTF_AUTO("Test: Erreur parsing '%s'", cmd.c_str());
    Serial.println("ERROR: Invalid command");
  }
}

bool GcodeParser::parseParameters(String params, MotionCommand &cmd) {
  cmd.has_x = cmd.has_y = cmd.has_z = cmd.has_e = cmd.has_f = cmd.has_s = false;
  params.trim();

  while (!params.isEmpty()) {
    int next_space = params.indexOf(' ');
    String param = (next_space == -1) ? params : params.substring(0, next_space);
    if (param.length() < 1) {
      DEBUG_PRINTF_AUTO("Erreur: Paramètre invalide '%s'", param.c_str());
      return false;
    }
    char param_type = param.charAt(0);
    float value = 0.0f;
    if (param.length() > 1) {
      value = param.substring(1).toFloat();
    }
    switch (param_type) {
      case 'X': cmd.x = value; cmd.has_x = true; break;
      case 'Y': cmd.y = value; cmd.has_y = true; break;
      case 'Z': cmd.z = value; cmd.has_z = true; break;
      case 'E': cmd.e = value; cmd.has_e = true; break;
      case 'F': cmd.f = value / 60.0; cmd.has_f = true; break; // Convertir mm/min en mm/s
      case 'S': cmd.s = value; cmd.has_s = true; break;
      default:
        DEBUG_PRINTF_AUTO("Erreur: Paramètre inconnu '%c'", param_type);
        return false;
    }
    params = (next_space == -1) ? "" : params.substring(next_space + 1);
    params.trim();
  }
  return true;
}

bool GcodeParser::parseLinearMovementCommand(String params, MotionCommand &cmd, GcodeType code) {
  cmd.type = 'G';
  cmd.code = static_cast<int>(code);
  if (!parseParameters(params, cmd)) return false;
  if (!cmd.has_x && !cmd.has_y && !cmd.has_z && !cmd.has_e) {
    DEBUG_PRINTF_AUTO("Erreur: G%d sans paramètres X, Y, Z, ou E", static_cast<int>(code));
    return false;
  }
  return true;
}

bool GcodeParser::parseArcMovementCommand(String params, MotionCommand &cmd, GcodeType code) {
  cmd.type = 'G';
  cmd.code = static_cast<int>(code);
  if (!parseParameters(params, cmd)) return false;
  if (!cmd.has_f || (!cmd.has_x && !cmd.has_y && !cmd.has_z)) {
    DEBUG_PRINTF_AUTO("Erreur: G%d (arc) sans F ou axes", static_cast<int>(code));
    return false;
  }
  return true;
}

bool GcodeParser::parseSetPositionCommand(String params, MotionCommand &cmd) {
  cmd.type = 'G';
  cmd.code = static_cast<int>(GcodeType::G92);
  if (!parseParameters(params, cmd)) return false;
  if (!cmd.has_x && !cmd.has_y && !cmd.has_z && !cmd.has_e) {
    DEBUG_PRINTF_AUTO("Erreur: G92 sans paramètres X, Y, Z, ou E");
    return false;
  }
  return true;
}

bool GcodeParser::parseHomingCommand(String params, MotionCommand &cmd) {
  cmd.type = 'G';
  cmd.code = static_cast<int>(GcodeType::G28);
  params.trim();
  if (params.isEmpty()) {
    cmd.has_x = cmd.has_y = cmd.has_z = true; // G28 sans paramètres = homing tous axes
    return true;
  }

  cmd.has_x = cmd.has_y = cmd.has_z = false;
  while (!params.isEmpty()) {
    int next_space = params.indexOf(' ');
    String param = (next_space == -1) ? params : params.substring(0, next_space);
    if (param.length() < 1) {
      DEBUG_PRINTF_AUTO("Erreur: Paramètre invalide '%s'", param.c_str());
      return false;
    }
    char param_type = param.charAt(0);
    if (param.length() > 1) {
      // Si une valeur est fournie (ex. X0), vérifier qu'elle est nulle
      float value = param.substring(1).toFloat();
      if (value != 0.0f) {
        DEBUG_PRINTF_AUTO("Erreur: G28 ne supporte pas de valeurs non nulles pour %c", param_type);
        return false;
      }
    }
    switch (param_type) {
      case 'X': cmd.has_x = true; break;
      case 'Y': cmd.has_y = true; break;
      case 'Z': cmd.has_z = true; break;
      default:
        DEBUG_PRINTF_AUTO("Erreur: Paramètre inconnu '%c' pour G28", param_type);
        return false;
    }
    params = (next_space == -1) ? "" : params.substring(next_space + 1);
    params.trim();
  }
  if (!cmd.has_x && !cmd.has_y && !cmd.has_z) {
    DEBUG_PRINTF_AUTO("Erreur: G28 sans axes spécifiés");
    return false;
  }
  return true;
}

bool GcodeParser::parseLevelingCommand(String params, MotionCommand &cmd) {
  cmd.type = 'G';
  cmd.code = static_cast<int>(GcodeType::G29);
  if (!parseParameters(params, cmd)) return false;
  // Validation spécifique pour leveling (à étendre si besoin)
  return true;
}

bool GcodeParser::parsePositioningCommand(String params, MotionCommand &cmd, GcodeType code) {
  cmd.type = 'G';
  cmd.code = static_cast<int>(code);
  if (!params.isEmpty()) {
    if (!parseParameters(params, cmd)) return false;
    if (cmd.has_x || cmd.has_y || cmd.has_z || cmd.has_e || cmd.has_f || cmd.has_s) {
      DEBUG_PRINTF_AUTO("Erreur: G%d ne doit pas avoir de paramètres", static_cast<int>(code));
      return false;
    }
  }
  if (code == GcodeType::G90) absolute_positioning = true;
  if (code == GcodeType::G91) absolute_positioning = false;
  if (code == GcodeType::G20 || code == GcodeType::G21) {
    // G20 (pouces) ou G21 (mm), à implémenter dans motion planner si besoin
  }
  return true;
}

bool GcodeParser::parseTemperatureCommand(String params, MotionCommand &cmd, GcodeType code) {
  cmd.type = 'M';
  cmd.code = static_cast<int>(code);
  if (!parseParameters(params, cmd)) return false;
  if (!cmd.has_s) {
    DEBUG_PRINTF_AUTO("Erreur: M%d nécessite un paramètre S", static_cast<int>(code) - 1000);
    return false;
  }
  if (cmd.has_x || cmd.has_y || cmd.has_z || cmd.has_e || cmd.has_f) {
    DEBUG_PRINTF_AUTO("Erreur: M%d ne doit pas avoir de paramètres X, Y, Z, E, ou F", static_cast<int>(code) - 1000);
    return false;
  }
  return true;
}

bool GcodeParser::parseFanCommand(String params, MotionCommand &cmd, GcodeType code) {
  cmd.type = 'M';
  cmd.code = static_cast<int>(code);
  if (code == GcodeType::M106) {
    if (!parseParameters(params, cmd)) return false;
    if (!cmd.has_s) {
      DEBUG_PRINTF_AUTO("Erreur: M106 nécessite un paramètre S");
      return false;
    }
  } else if (code == GcodeType::M107) {
    if (!params.isEmpty()) {
      if (!parseParameters(params, cmd)) return false;
      if (cmd.has_s) {
        DEBUG_PRINTF_AUTO("Erreur: M107 ne doit pas avoir de paramètre S");
        return false;
      }
    }
  }
  if (cmd.has_x || cmd.has_y || cmd.has_z || cmd.has_e || cmd.has_f) {
    DEBUG_PRINTF_AUTO("Erreur: M%d ne doit pas avoir de paramètres X, Y, Z, E, ou F", static_cast<int>(code) - 1000);
    return false;
  }
  return true;
}

bool GcodeParser::parseExtruderCommand(String params, MotionCommand &cmd, GcodeType code) {
  cmd.type = 'M';
  cmd.code = static_cast<int>(code);
  if (!params.isEmpty()) {
    if (!parseParameters(params, cmd)) return false;
    if (cmd.has_x || cmd.has_y || cmd.has_z || cmd.has_e || cmd.has_f || cmd.has_s) {
      DEBUG_PRINTF_AUTO("Erreur: M%d ne doit pas avoir de paramètres", static_cast<int>(code) - 1000);
      return false;
    }
  }
  if (code == GcodeType::M82) absolute_extrusion = true;
  if (code == GcodeType::M83) absolute_extrusion = false;
  return true;
}

bool GcodeParser::parseMotorsCommand(String params, MotionCommand &cmd, GcodeType code) {
  cmd.type = 'M';
  cmd.code = static_cast<int>(code);
  if (!parseParameters(params, cmd)) return false;
  // Validation spécifique pour motors (à étendre si besoin, ex. axes spécifiques)
  return true;
}

bool GcodeParser::parseSDCommand(String params, MotionCommand &cmd, GcodeType code) {
  cmd.type = 'M';
  cmd.code = static_cast<int>(code);
  if (!parseParameters(params, cmd)) return false;
  // Validation spécifique pour SD commands (ex. M23 nécessite S pour filename, à étendre si besoin)
  return true;
}

bool GcodeParser::parseReportingCommand(String params, MotionCommand &cmd, GcodeType code) {
  cmd.type = 'M';
  cmd.code = static_cast<int>(code);
  if (!params.isEmpty()) {
    if (!parseParameters(params, cmd)) return false;
    if (cmd.has_x || cmd.has_y || cmd.has_z || cmd.has_e || cmd.has_f || cmd.has_s) {
      DEBUG_PRINTF_AUTO("Erreur: M%d ne doit pas avoir de paramètres", static_cast<int>(code) - 1000);
      return false;
    }
  }
  return true;
}

bool GcodeParser::parseEmergencyCommand(String params, MotionCommand &cmd) {
  cmd.type = 'M';
  cmd.code = static_cast<int>(GcodeType::M112);
  if (!params.isEmpty()) {
    if (!parseParameters(params, cmd)) return false;
    if (cmd.has_x || cmd.has_y || cmd.has_z || cmd.has_e || cmd.has_f || cmd.has_s) {
      DEBUG_PRINTF_AUTO("Erreur: M112 ne doit pas avoir de paramètres");
      return false;
    }
  }
  return true;
}

void GcodeParser::parserTask(void *pvParameters) {
  String line;
  while (1) {
    if (xQueueReceive(gcodeQueue, &line, portMAX_DELAY) == pdTRUE) {
      DEBUG_PRINTF_AUTO("Parsing ligne: '%s'", line.c_str());
      MotionCommand cmd = {0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, false, false, false, false, false};
      int space_pos = line.indexOf(' ');
      String params = (space_pos == -1) ? "" : line.substring(space_pos + 1);
      String code_str = line.substring(0, space_pos == -1 ? line.length() : space_pos);

      if (!code_str.startsWith("G") && !code_str.startsWith("M")) {
        DEBUG_PRINTF_AUTO("Erreur: Type de commande inconnu '%s'", code_str.c_str());
        if (errorSemaphore) xSemaphoreGive(errorSemaphore);
        Serial.println("ERROR: Invalid command type");
        continue;
      }

      cmd.type = code_str.charAt(0);
      int raw_code = code_str.substring(1).toInt();
      cmd.code = raw_code;
      if (cmd.type == 'M') {
        cmd.code += 1000; // Décaler les M codes
      }

      bool valid = false;
      switch (static_cast<GcodeType>(cmd.code)) {
        case GcodeType::G0:
        case GcodeType::G1:
          valid = gcodeParser.parseLinearMovementCommand(params, cmd, static_cast<GcodeType>(cmd.code));
          break;
        case GcodeType::G2:
        case GcodeType::G3:
          valid = gcodeParser.parseArcMovementCommand(params, cmd, static_cast<GcodeType>(cmd.code));
          break;
        case GcodeType::G92:
          valid = gcodeParser.parseSetPositionCommand(params, cmd);
          break;
        case GcodeType::G28:
          valid = gcodeParser.parseHomingCommand(params, cmd);
          break;
        case GcodeType::G29:
          valid = gcodeParser.parseLevelingCommand(params, cmd);
          break;
        case GcodeType::G20:
        case GcodeType::G21:
        case GcodeType::G90:
        case GcodeType::G91:
          valid = gcodeParser.parsePositioningCommand(params, cmd, static_cast<GcodeType>(cmd.code));
          break;
        case GcodeType::M104:
        case GcodeType::M109:
        case GcodeType::M140:
        case GcodeType::M190:
          valid = gcodeParser.parseTemperatureCommand(params, cmd, static_cast<GcodeType>(cmd.code));
          break;
        case GcodeType::M106:
        case GcodeType::M107:
          valid = gcodeParser.parseFanCommand(params, cmd, static_cast<GcodeType>(cmd.code));
          break;
        case GcodeType::M82:
        case GcodeType::M83:
          valid = gcodeParser.parseExtruderCommand(params, cmd, static_cast<GcodeType>(cmd.code));
          break;
        case GcodeType::M17:
        case GcodeType::M18:
        case GcodeType::M84:
          valid = gcodeParser.parseMotorsCommand(params, cmd, static_cast<GcodeType>(cmd.code));
          break;
        case GcodeType::M20:
        case GcodeType::M21:
        case GcodeType::M22:
        case GcodeType::M23:
        case GcodeType::M24:
        case GcodeType::M25:
        case GcodeType::M26:
        case GcodeType::M27:
        case GcodeType::M28:
        case GcodeType::M29:
          valid = gcodeParser.parseSDCommand(params, cmd, static_cast<GcodeType>(cmd.code));
          break;
        case GcodeType::M105:
        case GcodeType::M114:
        case GcodeType::M115:
          valid = gcodeParser.parseReportingCommand(params, cmd, static_cast<GcodeType>(cmd.code));
          break;
        case GcodeType::M112:
          valid = gcodeParser.parseEmergencyCommand(params, cmd);
          break;
        default:
          DEBUG_PRINTF_AUTO("Erreur: Code %c%d non supporté", cmd.type, raw_code);
          if (errorSemaphore) xSemaphoreGive(errorSemaphore);
          Serial.println("ERROR: Unsupported command");
          continue;
      }

      if (valid) {
        if (xQueueSend(motionQueue, &cmd, pdMS_TO_TICKS(5000)) != pdTRUE) {
          DEBUG_PRINTF_AUTO("Erreur: Impossible d'envoyer à motionQueue après 5s");
          if (errorSemaphore) xSemaphoreGive(errorSemaphore);
          Serial.println("ERROR: Failed to send to motionQueue");
        } else {
          DEBUG_PRINTF_AUTO("Commande envoyée à motionQueue: %c%d", cmd.type, raw_code);
        }
      } else {
        DEBUG_PRINTF_AUTO("Erreur: Commande invalide '%s'", line.c_str());
        if (errorSemaphore) xSemaphoreGive(errorSemaphore);
        Serial.println("ERROR: Invalid command");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
