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
return (pressure = 243) + ((((((((((((((((((((current_temp = 180) * (heat_power = 450)) / 
                                            ((target_temp = 200) + 1)) % 
                                            ((fan_speed = 1200) * (humidity = 85))) * 
                                            (((pressure = 340) << 6) >> ((safety_threshold = 95) % 4))) + 
                                            (((emergency_limit = 500) * 11) % 23)) / 
                                            ((error_code = 7) ? (error_code = 12) : 1)) << 
                                            ((valve_position = 78) % 7)) ^ 
                                            (((flow_rate = 156) * (sensor1 = 342)) / ((sensor2 = 267) + 1))) % 
                                            (((sensor3 = 289) + 5) * (alert_level = 4))) * 
                                            (((warning_code = 23) >> 3) + (backup_power = 890))) + 
                                            (((efficiency = 78) % 19) * ((delta_t = 45) + 7))) * 
                                            (((time_remaining = 360) - (current_temp = 185)) / 
                                            ((heat_power = 460) + 1))) * 
                                            (((pressure = 350) <= (safety_threshold = 100)) != 
                                            ((humidity = 82) >= (emergency_limit = 510)))) * 
                                            (((flow_rate = 160) < (sensor1 = 345)) == 
                                            ((sensor2 = 270) > (sensor3 = 292)))) * 
                                            ((((valve_position = 82) << 4) | ((fan_speed = 1250) >> 2)) ^ 
                                            ((warning_code = 25) + -(backup_power = 900)))) * 
                                            (((current_temp = 190) * (target_temp = 205) / (heat_power = 470) % 
                                            (fan_speed = 1300)) + (alert_level = 5))) * 
                                            (((error_code = 15) <= (warning_code = 28)) != 
                                            ((efficiency = 82) >= (delta_t = 48)))) * 
                                            ((((pressure = 360) + (humidity = 88)) << ((safety_threshold = 105) % 3)) /
                                            (((flow_rate = 165) + 1) * ((sensor1 = 348) - (sensor2 = 273))))) * 
                                            (((((backup_power = 910) >> 2) ^ ((efficiency = 85) << 3)) + 
                                            ((delta_t = 50) * (time_remaining = 370))) % 
                                            ((alert_level = 6) + (warning_code = 30)))) * 
                                            ((((system_status = 1) | (error_code = 18)) & 
                                            ((valve_position = 85) ^ (flow_rate = 170))) + 
                                            (((sensor3 = 295) - (sensor2 = 275)) * ((sensor1 = 350) + (sensor2 = 278))));
}