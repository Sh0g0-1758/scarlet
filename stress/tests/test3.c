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

start:
  if ((packet_size = 1024) > 0) {
    if ((header_length = packet_size / 8) < 256) {
      payload_size = 960;
      goto compute_checksum;
    } else {
      payload_size = 512;
      goto compute_buffer;
    }
  } else {
    header_length = 0;
    goto error_handler;
  }

compute_checksum:
  if ((checksum = header_length * 2) != 0) {
    error_bits = checksum + 16;
    if (error_bits > 100)
      goto compute_buffer;
    else
      goto process_packet;
  }
  goto error_handler;

compute_buffer:
  buffer_space = error_bits * 4;
  if (buffer_space < 1024)
    goto process_packet;
  else {
    buffer_space = buffer_space / 2;
    goto process_packet;
  }

process_packet:
  packet_id = (payload_size + checksum) * 2;
  if (packet_id > 2000) {
    queue_length = packet_id / 4;
    goto handle_queue;
  } else {
    queue_length = packet_id * 2;
    if (queue_length < 1000)
      goto error_handler;
    goto handle_queue;
  }

handle_queue:
  timeout = packet_id + queue_length;
  if (timeout > 5000) {
    retry_count = timeout / 2;
    max_retries = retry_count - 100;
    goto fragment_packet;
  } else {
    retry_count = timeout * 2;
    max_retries = retry_count + 50;
    if (max_retries > 7000)
      goto error_handler;
    goto fragment_packet;
  }

fragment_packet:
  fragment_size = retry_count + max_retries;
  total_fragments = fragment_size - 200;
  if (fragment_size > total_fragments) {
    processed_count = fragment_size / 2;
    dropped_count = total_fragments * 2;
    goto compute_stats;
  } else {
    goto error_handler;
  }

compute_stats:
  bandwidth = processed_count + dropped_count;
  latency = bandwidth - 100;
  goto final;

error_handler:
  return -1;

final:
  return --buffer_space +
         ((((((((((((((((((packet_size * fragment_size) / (header_length + 1)) %
                         (payload_size - checksum)) *
                        ((error_bits << 4) >> 2)) +
                       ((packet_id * 7) % 13)) /
                      (queue_length ? queue_length : 1))
                     << (timeout % 6)) ^
                    (retry_count * max_retries)) %
                   ((delay + 3) * fragment_size)) *
                  (total_fragments >> 3)) +
                 ((processed_count % 17) * (dropped_count + 5))) *
                ((bandwidth - latency) / (packet_size + 1))) *
               ((error_bits <= 5) != (checksum >= payload_size))) *
              ((packet_id < total_fragments) ==
               (buffer_space > queue_length))) *
             (((timeout << 5) | (delay >> 3)) ^ (latency + -bandwidth))) *
            ((packet_size * header_length / payload_size % fragment_size) +
             dropped_count)) *
           ((retry_count <= max_retries) !=
            (processed_count >= total_fragments))));
}
