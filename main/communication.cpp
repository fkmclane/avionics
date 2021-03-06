/*
 * Definition file for communication functions
 */

/* LIBRARIES */
#include <Arduino.h>

#include "pins.h"
#include "datalog.h"
#include "ninedof.h"
#include "barometer.h"
#include "gps.h"
#include "communication.h"
#include "sim.h"
#include "json.h"

const char * commands_arr[] = {
  "none",
  "test",
  "arm",
  "disarm",
  "ignite",
  "abort",
  "pass",
  "fail",
  ""
};

const char ** commands = commands_arr;

DynamicJsonBuffer json;

JsonObject & msg_error = json.createObject();
JsonObject & msg_telemetry = json.createObject();
JsonObject & msg_result = json.createObject();
JsonObject & msg_state = json.createObject();

enum state_e last_state;

void communication_sendResult(bool pass) {
  msg_result["type"] = "result";
  msg_result["time"] = millis();
  msg_result["pass"] = pass;

  msg_result.printTo(Serial2);
  Serial2.println();

  String str;
  msg_result.printTo(str);
  datalog_write(str);

  delay(COMM_DELAY);
  Serial2.flush();
}

void communication_sendState(enum state_e state) {
  last_state = state;

  msg_state["type"] = "state";
  msg_state["time"] = millis();
  msg_state["state"] = states[last_state];

  msg_state.printTo(Serial2);
  Serial2.println();

  String str;
  msg_state.printTo(str);
  datalog_write(str);

  delay(COMM_DELAY);
  Serial2.flush();
}

enum command_e communication_recvCommand() {
  static unsigned int comm = COMM_COUNT;

  if (Serial2.available()) {
    comm = 0;

    JsonObject & command = json.parseObject(Serial2);

    for (unsigned int idx = 0; idx < sizeof(commands_arr)/sizeof(commands_arr[0]); idx++) {
      if (commands[idx] == command["command"])
        return (enum command_e)idx;
    }

    return NO_COMM;
  }
  else if (comm >= COMM_COUNT) {
    return NO_COMM;
  }
  else {
    comm++;

    return CMD_NONE;
  }
}

void communication_updateTelemetry() {
  ninedof_read(true);
  barometer_read(true);
  gps_read();

#ifdef SIM
  sim_updateTelemetry();
#endif

  msg_telemetry["type"] = "telemetry";
  msg_telemetry["time"] = millis();
  msg_telemetry["state"] = states[last_state];

  msg_telemetry["sensors"]["gyro"]["x"] = gyro.x;
  msg_telemetry["sensors"]["gyro"]["y"] = gyro.y;
  msg_telemetry["sensors"]["gyro"]["z"] = gyro.z;

  msg_telemetry["sensors"]["acc"]["x"] = acc.x;
  msg_telemetry["sensors"]["acc"]["y"] = acc.y;
  msg_telemetry["sensors"]["acc"]["z"] = acc.z;

  msg_telemetry["sensors"]["mag"]["x"] = mag.x;
  msg_telemetry["sensors"]["mag"]["y"] = mag.y;
  msg_telemetry["sensors"]["mag"]["z"] = mag.z;

  msg_telemetry["sensors"]["att"]["r"] = att.r;
  msg_telemetry["sensors"]["att"]["p"] = att.p;
  msg_telemetry["sensors"]["att"]["y"] = att.y;

  msg_telemetry["sensors"]["bar"]["p"] = bar.p;
  msg_telemetry["sensors"]["bar"]["dp"] = bar.dp;

  msg_telemetry["sensors"]["bar"]["alt"] = bar.alt;
  msg_telemetry["sensors"]["bar"]["gnd"] = bar.gnd;

  msg_telemetry["sensors"]["bar"]["temp"] = bar.temp;
  msg_telemetry["sensors"]["bar"]["hum"] = bar.hum;

  msg_telemetry["sensors"]["gps"]["lat"] = gps.lat;
  msg_telemetry["sensors"]["gps"]["lon"] = gps.lon;

  msg_telemetry["sensors"]["gps"]["lat"] = gps.lat;
  msg_telemetry["sensors"]["gps"]["lon"] = gps.lon;

  msg_telemetry["sensors"]["gps"]["hour"] = gps.hour;
  msg_telemetry["sensors"]["gps"]["min"] = gps.min;
  msg_telemetry["sensors"]["gps"]["sec"] = gps.sec;

  msg_telemetry["sensors"]["gps"]["day"] = gps.day;
  msg_telemetry["sensors"]["gps"]["mon"] = gps.mon;
  msg_telemetry["sensors"]["gps"]["year"] = gps.year;

  msg_telemetry.printTo(Serial2);
  Serial2.println();

  String str;
  msg_telemetry.printTo(str);
  datalog_write(str);

  delay(COMM_DELAY);
  Serial2.flush();
}

void communication_init() {
  msg_error["type"] = "error";
  msg_error["time"] = millis();
  msg_error["message"] = "none";

  JsonObject & sensors = msg_telemetry.createNestedObject("sensors");
  sensors.createNestedObject("gyro");
  sensors.createNestedObject("acc");
  sensors.createNestedObject("mag");
  sensors.createNestedObject("att");
  sensors.createNestedObject("bar");
  sensors.createNestedObject("gps");

  msg_telemetry["type"] = "telemetry";
  msg_telemetry["time"] = millis();
  msg_telemetry["state"] = "init";

  msg_telemetry["sensors"]["gyro"]["x"] = 0;
  msg_telemetry["sensors"]["gyro"]["y"] = 0;
  msg_telemetry["sensors"]["gyro"]["z"] = 0;

  msg_telemetry["sensors"]["acc"]["x"] = 0;
  msg_telemetry["sensors"]["acc"]["y"] = 0;
  msg_telemetry["sensors"]["acc"]["z"] = 0;

  msg_telemetry["sensors"]["mag"]["x"] = 0;
  msg_telemetry["sensors"]["mag"]["y"] = 0;
  msg_telemetry["sensors"]["mag"]["z"] = 0;

  msg_telemetry["sensors"]["att"]["r"] = 0;
  msg_telemetry["sensors"]["att"]["p"] = 0;
  msg_telemetry["sensors"]["att"]["y"] = 0;

  msg_telemetry["sensors"]["bar"]["p"] = 0;
  msg_telemetry["sensors"]["bar"]["dp"] = 0;
  msg_telemetry["sensors"]["bar"]["alt"] = 0;
  msg_telemetry["sensors"]["bar"]["gnd"] = 0;
  msg_telemetry["sensors"]["bar"]["temp"] = 0;
  msg_telemetry["sensors"]["bar"]["hum"] = 0;

  msg_telemetry["sensors"]["gps"]["lat"] = 0;
  msg_telemetry["sensors"]["gps"]["lon"] = 0;

  msg_telemetry["sensors"]["gps"]["hour"] = 0;
  msg_telemetry["sensors"]["gps"]["min"] = 0;
  msg_telemetry["sensors"]["gps"]["sec"] = 0;

  msg_telemetry["sensors"]["gps"]["day"] = 0;
  msg_telemetry["sensors"]["gps"]["mon"] = 0;
  msg_telemetry["sensors"]["gps"]["year"] = 0;

  msg_result["type"] = "result";
  msg_result["time"] = millis();
  msg_result["pass"] = false;

  msg_state["type"] = "state";
  msg_state["time"] = millis();
  msg_state["state"] = "init";

  Serial2.begin(115200);
}
