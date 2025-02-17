int main(void) {
  int packet_size = 0;
  int header_length = 0;
  int payload_size = 0;
  int checksum = 0;
  int error_bits = 0;
  int packet_id = 0;
  int buffer_space = 0;
  int queue_length = 0;
  int timeout = 0;
  int retry_count = 0;
  int max_retries = 0;
  int delay = 0;
  int fragment_size = 0;
  int total_fragments = 0;
  int processed_count = 0;
  int dropped_count = 0;
  int bandwidth = 0;
  int latency = 0;
singularity_start:
  if (((packet_size = 981) >> 3) + ((header_length = 129) << 4) >
      ((payload_size = 9) ^ (checksum = 8))) {
    if ((((error_bits = packet_size * 31) % 61) +
         ((packet_id = header_length >> 5) * 23)) <
        ((buffer_space = payload_size ^ checksum) + 7)) {
      queue_length = ((timeout = 6) & (retry_count = 5)) |
                     ((max_retries = 4) ^ (delay = 3));
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
        ((latency = packet_size ^ 13) + 1)) {
      goto entropy_path;
    }
    goto chaos_path;
  }
  goto void_path;

entropy_path:
  if ((((error_bits = packet_id * 41) >> 7) +
       ((buffer_space = queue_length + 97) << 3)) >
      ((timeout = retry_count ^ max_retries) - 2)) {
    if ((((delay = fragment_size * 43) % 71) *
         ((total_fragments = processed_count + 83) >> 5)) !=
        ((dropped_count = bandwidth & latency) + 3)) {
      goto chaos_path;
    }
    goto final_collapse;
  }
  goto quantum_path;

chaos_path:
  if ((((packet_size = header_length * 4) % 789) +
       ((payload_size = checksum + 89) << 6)) <
      ((error_bits = packet_id ^ buffer_space) - 3454)) {
    if ((((queue_length = timeout * 53) >> 4) *
         ((retry_count = max_retries + 79) % 67)) >
        ((delay = fragment_size & total_fragments) + 5)) {
      goto void_path;
    }
  }
  goto final_collapse;

void_path:
  if ((((processed_count = dropped_count * 59) % 79) +
       ((bandwidth = latency + 101) << 5)) !=
      ((packet_size = header_length ^ payload_size) - 6)) {
    if ((((checksum = error_bits * 61) >> 6) *
         ((packet_id = buffer_space + 103) % 83)) <
        ((queue_length = timeout & retry_count) + 7)) {
      goto final_collapse;
    }
    goto singularity_start;
  }
  goto chaos_path;

final_collapse:
  packet_size = 0;
  return (
      (((packet_size * header_length) << 11) +
       ((payload_size ^ checksum) >> 7)) *
          ((error_bits & packet_id) | ((buffer_space + queue_length) % 97)) +
      ((timeout * retry_count) - ((max_retries ^ delay) << 5)) *
          ((fragment_size & total_fragments) |
           ((processed_count + dropped_count) % 89)) +
      ((bandwidth * latency) ^ ((packet_size - header_length) << 3)));
}
