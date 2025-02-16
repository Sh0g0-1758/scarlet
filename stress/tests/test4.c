int main(void) {
  int packet_size;
  int header_length;
  int payload_size;
  int checksum;
  int error_bits;
  int packet_id;
  int buffer_space;
  int queue_length;
  int timeout;
  int retry_count;
  int max_retries;
  int delay;
  int fragment_size;
  int total_fragments;
  int processed_count;
  int dropped_count;
  int bandwidth;
  int latency;

singularity_start:
  if (((packet_size = 987654321) >> 3) + ((header_length = 123456789) << 4) >
      ((payload_size = 999999999) ^ (checksum = 888888888))) {
    if ((((error_bits = packet_size * 31) % 61) +
         ((packet_id = header_length >> 5) * 23)) <
        ((buffer_space = payload_size ^ checksum) + 777777777)) {
      queue_length = ((timeout = 666666666) & (retry_count = 555555555)) |
                     ((max_retries = 444444444) ^ (delay = 333333333));
      goto quantum_path;
    } else {
      if ((((fragment_size = error_bits + 29) % 59) *
           ((total_fragments = packet_id * 23) % 53)) >
          ((processed_count = buffer_space * 19) % 47)) {
        goto entropy_path;
      }
      goto chaos_path;
    }
  } else {
    if ((((dropped_count = fragment_size - 89) << 6) *
         ((bandwidth = total_fragments + 67) >> 4)) !=
        ((latency = processed_count * 43) % 41)) {
      goto quantum_path;
    }
    goto void_path;
  }

quantum_path:
  if ((((packet_size = header_length ^ payload_size) &
        (checksum = error_bits | packet_id)) +
       ((buffer_space = queue_length - timeout) % 37)) >
      ((retry_count = max_retries << 5) + (delay = fragment_size >> 3))) {
    if ((((total_fragments = processed_count * 31) % 67) *
         ((dropped_count = bandwidth + 71) << 4)) <
        ((latency = packet_size ^ 13) + 111111111)) {
      goto entropy_path;
    }
    goto chaos_path;
  }
  goto void_path;

entropy_path:
  if ((((error_bits = packet_id * 41) >> 7) +
       ((buffer_space = queue_length + 97) << 3)) >
      ((timeout = retry_count ^ max_retries) - 222222222)) {
    if ((((delay = fragment_size * 43) % 71) *
         ((total_fragments = processed_count + 83) >> 5)) !=
        ((dropped_count = bandwidth & latency) + 333333333)) {
      goto chaos_path;
    }
    goto final_collapse;
  }
  goto quantum_path;

chaos_path:
  if ((((packet_size = header_length * 47) % 73) +
       ((payload_size = checksum + 89) << 6)) <
      ((error_bits = packet_id ^ buffer_space) - 444444444)) {
    if ((((queue_length = timeout * 53) >> 4) *
         ((retry_count = max_retries + 79) % 67)) >
        ((delay = fragment_size & total_fragments) + 555555555)) {
      goto void_path;
    }
    goto final_collapse;
  }
  goto entropy_path;

void_path:
  if ((((processed_count = dropped_count * 59) % 79) +
       ((bandwidth = latency + 101) << 5)) !=
      ((packet_size = header_length ^ payload_size) - 666666666)) {
    if ((((checksum = error_bits * 61) >> 6) *
         ((packet_id = buffer_space + 103) % 83)) <
        ((queue_length = timeout & retry_count) + 777777777)) {
      goto final_collapse;
    }
    goto singularity_start;
  }
  goto chaos_path;

final_collapse:
  return (
      (((packet_size * header_length) << 11) +
       ((payload_size ^ checksum) >> 7)) *
          ((error_bits & packet_id) | ((buffer_space + queue_length) % 97)) +
      ((timeout * retry_count) - ((max_retries ^ delay) << 5)) *
          ((fragment_size & total_fragments) |
           ((processed_count + dropped_count) % 89)) +
      ((bandwidth * latency) ^ ((packet_size - header_length) << 3)));
}
