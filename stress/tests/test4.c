int main(void) {
  int balance;
  int transaction_amount;
  int fee;
  int daily_limit;
  int remaining_limit;
  int transaction_count;
  int max_transactions;
  int approval_code;
  int verification_id;
  int risk_score;
  int threshold;
  int processed_amount;
  int pending_amount;
  int account_age;
  int credit_score;
  int reward_points;
  int bonus_multiplier;
  int final_amount;
  int total_fees;

start:
  if ((balance = 10000) > 0) {
    if ((transaction_amount = balance / 10) < 2000) {
      fee = 25;
      goto check_limit;
    } else {
      fee = 50;
      goto compute_risk;
    }
  } else {
    transaction_amount = 0;
    goto reject;
  }

check_limit:
  if ((daily_limit = transaction_amount * 5) != 0) {
    remaining_limit = daily_limit - fee;
    if (remaining_limit > 5000)
      goto compute_risk;
    else
      goto process_transaction;
  }
  goto reject;

compute_risk:
  risk_score = remaining_limit * 2;
  if (risk_score < 8000)
    goto process_transaction;
  else {
    risk_score = risk_score / 2;
    goto process_transaction;
  }

process_transaction:
  transaction_count = (fee + remaining_limit) * 2;
  if (transaction_count > 1000) {
    max_transactions = transaction_count / 2;
    goto verify;
  } else {
    max_transactions = transaction_count * 2;
    if (max_transactions < 500)
      goto reject;
    goto verify;
  }

verify:
  approval_code = transaction_count + max_transactions;
  if (approval_code > 2000) {
    verification_id = approval_code / 2;
    threshold = verification_id - 500;
    goto compute_rewards;
  } else {
    verification_id = approval_code * 2;
    threshold = verification_id + 250;
    if (threshold > 3000)
      goto reject;
    goto compute_rewards;
  }

compute_rewards:
  processed_amount = verification_id + threshold;
  pending_amount = processed_amount - 1000;
  if (processed_amount > pending_amount) {
    reward_points = processed_amount / 2;
    bonus_multiplier = pending_amount * 2;
    goto finalize;
  } else {
    goto reject;
  }

finalize:
  final_amount = reward_points + bonus_multiplier;
  total_fees = final_amount - 500;
  goto final;

reject:
  return -1;

final:
  return (!(~balance & 255) &&
              (((transaction_count << 2) | (max_transactions >> 1)) ^
               (approval_code + -fee)) > (risk_score * threshold /
                                          processed_amount % pending_amount) ||
          (reward_points <= bonus_multiplier) !=
              (final_amount >= total_fees)) &&
         ((processed_amount < pending_amount) == (final_amount > total_fees));
}
