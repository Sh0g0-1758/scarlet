int main(void) {
  int player_health;
  int enemy_health;
  int damage;
  int weapon_power;
  int armor_value;
  int shield;
  int experience;
  int level;
  int skill_points;
  int position_x;
  int position_y;
  int speed;
  int inventory_size;
  int item_count;
  int combo_multiplier;
  int critical_chance;
  int score;
  int high_score;

start:
  if ((player_health = 100) > 0) {
    if ((enemy_health = player_health * 2) < 300) {
      damage = 25;
      goto compute_defense;
    } else {
      damage = 50;
      goto compute_attack;
    }
  } else {
    enemy_health = 0;
    goto game_over;
  }

compute_defense:
  if ((armor_value = damage * 3) != 0) {
    shield = armor_value + 15;
    if (shield > 100)
      goto compute_attack;
    else
      goto update_stats;
  }
  goto game_over;

compute_attack:
  weapon_power = shield * 2;
  if (weapon_power < 200)
    goto update_stats;
  else {
    weapon_power = weapon_power / 2;
    goto update_stats;
  }

update_stats:
  experience = (damage + shield) * 2;
  if (experience > 500) {
    level = experience / 5;
    goto update_position;
  } else {
    level = experience * 2;
    if (level < 200)
      goto game_over;
    goto update_position;
  }

update_position:
  position_x = experience + level;
  if (position_x > 1000) {
    position_y = position_x / 2;
    speed = position_y - 200;
    goto compute_inventory;
  } else {
    position_y = position_x * 2;
    speed = position_y + 100;
    if (speed > 2000)
      goto game_over;
    goto compute_inventory;
  }

compute_inventory:
  inventory_size = position_x + position_y;
  item_count = inventory_size - 500;
  if (inventory_size > item_count) {
    combo_multiplier = inventory_size / 2;
    critical_chance = item_count * 2;
    goto compute_score;
  } else {
    goto game_over;
  }

compute_score:
  score = combo_multiplier + critical_chance;
  high_score = score - 250;
  goto final;

game_over:
  return -1;

final:
  return --player_health +
         ((((((((((((((((enemy_health + damage) * weapon_power) / armor_value) %
                      shield) *
                     inventory_size) *
                    combo_multiplier) <= critical_chance) !=
                  (score >= high_score)) *
                 ((position_x < position_y) == (score > high_score))) *
                (player_health & 5)) *
               (((experience << 7) | (level >> 17)) ^ (position_x + -damage))) *
              (weapon_power * armor_value / shield % inventory_size)) *
                 (combo_multiplier <= critical_chance) !=
             (score >= high_score)) *
            ((position_x < position_y) == (score > high_score)))));
}
