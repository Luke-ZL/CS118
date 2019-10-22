
//
//  macros.h
//  
//
//  Created by Lihang Liu on 6/2/19.
//

#ifndef macros_h
#define macros_h

#define PACKET_SIZE 524
#define PAYLOAD 512
#define HEADER 12
#define RTO 0.5

#define MAX_SEQ_NUM 25600
#define ACK_IF_NOT_SET 0

#define INIT_CWND 512
#define MAX_CWND 10240
#define INIT_SSTHRESH 5120
#define EC_FR 1536

#define ACK 0b00000001
#define SYN 0b00000010
#define FIN 0b00000100

#endif /* macros_h */
