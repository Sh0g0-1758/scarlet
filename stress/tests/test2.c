int main(void) {
  int current_temp;
  int target_temp;
  int heat_power;
  int fan_speed;
  int humidity;
  int pressure;
  int safety_threshold;
  int emergency_limit;
  int system_status;
  int error_code;
  int valve_position;
  int flow_rate;
  int sensor1;
  int sensor2;
  int sensor3;
  int alert_level;
  int warning_code;
  int backup_power;
  int efficiency;
  int delta_t;
  int time_remaining;

start:
  if ((current_temp = 72) < 100) {
    if ((target_temp = current_temp + 5) > 65) {
      heat_power = 80;
      goto check_fan;
    } else {
      heat_power = 40;
      goto check_pressure;
    }
  } else {
    heat_power = 0;
    goto emergency;
  }

check_fan:
  if ((fan_speed = heat_power / 2) > 0) {
    humidity = fan_speed + 30;
    if (humidity > 60)
      goto check_pressure;
    else
      goto check_sensors;
  }
  goto emergency;

check_pressure:
  pressure = humidity * 2;
  if (pressure < 120)
    goto check_sensors;
  else {
    pressure = pressure / 2;
    goto check_sensors;
  }

check_sensors:
  sensor1 = (heat_power + fan_speed) * 2;
  if (sensor1 > 200) {
    sensor2 = sensor1 / 2;
    goto compute_status;
  } else {
    sensor2 = sensor1 * 2;
    if (sensor2 < 100)
      goto emergency;
    goto compute_status;
  }

compute_status:
  sensor3 = sensor1 + sensor2;
  if (sensor3 > 300) {
    system_status = sensor3 / 2;
    error_code = system_status - 100;
    goto check_valves;
  } else {
    system_status = sensor3 * 2;
    error_code = system_status + 50;
    if (error_code > 500)
      goto emergency;
    goto check_valves;
  }

check_valves:
  valve_position = system_status + error_code;
  flow_rate = valve_position - 200;
  if (valve_position > flow_rate) {
    alert_level = valve_position / 2;
    warning_code = flow_rate * 2;
    goto compute_efficiency;
  } else {
    goto emergency;
  }

compute_efficiency:
  backup_power = alert_level + warning_code;
  efficiency = backup_power - 75;
  goto final;

emergency:
  return -1;

final:
  return (!(~current_temp & 255) &&
              (((sensor1 << 2) | (sensor2 >> 1)) ^ (sensor3 + -heat_power)) >
                  (pressure * fan_speed / humidity % flow_rate) ||
          (system_status <= error_code) != (valve_position >= flow_rate)) &&
         ((alert_level < warning_code) == (backup_power > efficiency));
}