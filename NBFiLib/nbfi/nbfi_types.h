#ifndef NBFI_TYPES_H
#define NBFI_TYPES_H


// NB-Fi flags
#define SYS_FLAG        (1<<7)
#define ACK_FLAG        (1<<6)
#define MULTI_FLAG      (1<<5)

// NB-Fi system packet types
#define SYSTEM_PACKET_ACK               0x00
#define SYSTEM_PACKET_HERTBEAT          0x01
#define SYSTEM_PACKET_GROUP_START       0x02
#define SYSTEM_PACKET_ACK_ON_SYS        0x03
#define SYSTEM_PACKET_CLEAR             0x04
#define SYSTEM_PACKET_GROUP_START_OLD   0x05
#define SYSTEM_PACKET_CONFIG            0x06
#define SYSTEM_PACKET_RESET             0x07
#define SYSTEM_PACKET_CLEAR_EXT         0x08
#define SYSTEM_PACKET_TIME              0x09
#define SYSTEM_PACKET_SYNC              0x0A

#define NBFI_PACKET_SIZE        8 //neccessary if no malloc used

#define NBFI_ALTERNATIVES_NUMBER    8

/*NBFi transport layer frame struct*/
typedef struct
{
    union
    {
        struct
        {
            uint8_t ITER        : 5;//LSB
            uint8_t MULTI       : 1;
            uint8_t ACK         : 1;
            uint8_t SYSTEM      : 1;//MSB
        };
        uint8_t header;
    };

#ifdef NBFI_USE_MALLOC
    uint8_t payload[0];     //begining of packet payload
#else
    uint8_t payload[NBFI_PACKET_SIZE];     //begining of packet payload
#endif
}nbfi_transport_frame_t;

typedef enum
{
    HANDSHAKE_NONE      = 0,
    HANDSHAKE_SIMPLE    = 1
}nbfi_handshake_t;

typedef enum
{
    PACKET_FREE             = 0,
    PACKET_ALLOCATED        = 1,
    PACKET_QUEUED           = 2,
    PACKET_QUEUED_AGAIN     = 3,
    PACKET_SENT             = 5,
    PACKET_SENT_NOACKED     = 6,
    PACKET_RECEIVED         = 7,
    PACKET_DELIVERED        = 8,
    PACKET_LOST             = 9,
    PACKET_PROCESSING       = 0x0A,
    PACKET_PROCESSED        = 0x0B,
    PACKET_WAIT_ACK         = 0x10,
    PACKET_NEED_TO_SEND_RIGHT_NOW   = 0x13,
    PACKET_WAIT_FOR_EXTRA_PACKETS   = 0x14,
    PACKET_CLEARED          = 0x15
}nbfi_packet_state_t;


/*NBFi transport layer struct*/

typedef struct
{
    nbfi_packet_state_t state;              //packet state
    uint8_t             id;		    //ulapp id
    uint32_t		ts;		    //timestamp of packet
    nbfi_handshake_t    handshake;          //packet handshake mode
    uint8_t             retry_num;          //retry counter
    uint8_t             mack_num;           //number of packets for multi ack mode
    uint8_t             phy_data_length;    //length of packet payload(without header)
    nbfi_transport_frame_t   phy_data;      //transport layer frame data
}nbfi_transport_packet_t;

typedef enum
{
    NRX         =   0,
    DRX         =   1,
    CRX         =   2,
    OFF         =   4
}nbfi_mode_t;

typedef enum
{
    MACK_0      = 0,//no ack
    MACK_1      = 1,//0x1,
    MACK_2      = 2,//0x3,
    MACK_4      = 4,//0x0f,
    MACK_8      = 8,//0xff,
    MACK_16     = 16,//0xffff,
    MACK_32     = 32,//0,
}nbfi_mack_mode_t;

typedef enum
{
    DL_PSK_200              = 0,
    DL_PSK_500              = 1,
    DL_PSK_5000             = 2,
    DL_PSK_FASTDL           = 3,
    DL_DBPSK_50_PROT_D      = 10,
    DL_DBPSK_400_PROT_D	    = 11,
    DL_DBPSK_3200_PROT_D    = 12,
    DL_DBPSK_25600_PROT_D   = 13,
    DL_DBPSK_50_PROT_E	    = 14,
    DL_DBPSK_400_PROT_E	    = 15,
    DL_DBPSK_3200_PROT_E    = 16,
    DL_DBPSK_25600_PROT_E   = 17,
    DL_DBPSK_100H_PROT_D    = 18,
    UL_DBPSK_50_PROT_C      = 20,
    UL_DBPSK_50_PROT_D      = 21,
    UL_PSK_200              = 22,
    UL_DBPSK_400_PROT_C     = 23,
    UL_DBPSK_400_PROT_D     = 24,
    UL_PSK_500              = 25,
    UL_DBPSK_3200_PROT_D    = 26,
    UL_PSK_5000             = 27,
    UL_DBPSK_25600_PROT_D   = 28,
    UL_PSK_FASTDL           = 29,
    UL_DBPSK_50_PROT_E	    = 30,
    UL_DBPSK_400_PROT_E	    = 31,
    UL_DBPSK_3200_PROT_E    = 32,
    UL_DBPSK_25600_PROT_E   = 33,
    UL_DBPSK_100H_PROT_D    = 34,
    UL_DBPSK_100H_PROT_E    = 35, 
    UL_CARRIER              = 50,
    DL_RS485_PROT_D         = 60,
    UNDEFINED               = 100
}nbfi_phy_channel_t;


typedef enum
{
    PROT_AXSEM              = 0,
    PROT_C                  = 1,
    PROT_D                  = 2,
    PROT_E                  = 3,
    PROT_VOID               = 100
}nbfi_prot_t;

typedef struct
{
    uint32_t UL_total;
    uint32_t UL_iter;
    uint32_t DL_total;
    uint32_t DL_iter;
    uint8_t  aver_rx_snr;
    uint8_t  aver_tx_snr;
    int16_t success_total;
    int16_t fault_total;
    uint32_t last_tx_freq;
    uint32_t last_rx_freq;
    int16_t last_rssi;
    uint8_t last_snr;
    uint8_t UL_rating;
    uint8_t DL_rating;
    uint32_t DL_last_time;
    uint32_t bs_id;
    uint32_t server_id;
    uint32_t mkA_hours_consumed_tx;
    uint32_t mkA_hours_consumed_rx;
    
}nbfi_state_t;


typedef union
{
  uint16_t fp;
  struct
  {
    uint16_t dl_offset      : 3;//LSB
    uint16_t dl_sign        : 1;
    uint16_t dl_width       : 2;
    uint16_t ul_offset      : 6;
    uint16_t ul_sign        : 1;
    uint16_t ul_width       : 3;//MSB
  };
} nbfi_freq_plan_t;

typedef struct
{
    uint8_t try_interval;
    nbfi_phy_channel_t	try_tx_phy_channel;
    nbfi_phy_channel_t 	try_rx_phy_channel;
    nbfi_freq_plan_t try_nbfi_freq_plan;
}nbfi_try_alternative_t;


typedef enum
{
    OK = 0,
    ERR = 1,
    ERR_RF_BUSY = 2,
    ERR_BUFFER_FULL_v4 = 4
}nbfi_status_t;


typedef enum
{
    PCB = 0,       //PCB or ANT 1
    SMA = 1        //SMA or ANT 2
}nbfi_rf_antenna_t;

typedef struct
{
    uint32_t            *modem_id;
    uint32_t            *master_key;
    nbfi_mode_t 	mode;
    nbfi_phy_channel_t	tx_phy_channel;
    nbfi_phy_channel_t 	rx_phy_channel;
    nbfi_handshake_t	handshake_mode;
    nbfi_mack_mode_t	mack_mode;
    uint8_t             num_of_retries;
    uint8_t             max_payload_len;
    uint16_t            wait_ack_timeout;
    uint32_t            tx_freq;
    uint32_t            rx_freq;
    uint8_t             tx_antenna;
    uint8_t             rx_antenna;
    int8_t              tx_pwr;
    uint16_t            heartbeat_interval;
    uint8_t             heartbeat_num;
    uint16_t            additional_flags;
    uint32_t            ul_freq_base;
    uint32_t            dl_freq_base;
    nbfi_freq_plan_t    nbfi_freq_plan;
    nbfi_try_alternative_t try_alternative[NBFI_ALTERNATIVES_NUMBER];
}nbfi_settings_t;


typedef struct
{
	uint32_t ul;
	uint32_t dl;
}nbfi_crypto_iterator_t;

typedef struct
{
	//uint32_t* modem_id;
	//uint32_t* key;
	int8_t tx_min_pwr;
	int8_t tx_max_pwr;
	uint16_t manufacturer_id;
	uint16_t hardware_type_id;
	uint16_t protocol_id;
	uint8_t band_id;
	uint32_t send_info_interval;
}nbfi_dev_info_t;

typedef enum
{
    DOWN = 0,     // data rate change down direction
    UP = 1        // data rate change up direction
}nbfi_rate_direct_t;


typedef struct
{

  union
  {
          uint8_t byte;
          struct
          {
              uint8_t RTC_MSB          : 6;//LSB
              uint8_t DL_SPEED_NOT_MAX : 1;
              uint8_t UL_SPEED_NOT_MAX : 1;
          };
  } info;
  nbfi_freq_plan_t fp;
} NBFi_station_info_s;


typedef enum
{
  NOTEXIST = 0,
  QUEUED = 1,
  INPROCESS = 2,
  DELIVERED = 3,
  LOST = 4,
  ERR_BUFFER_FULL = 5,
  ERR_PACKET_IS_TOO_LONG = 6
}nbfi_ul_status_t;

//ULAPP flags:
#define NBFI_UL_FLAG_NOACK                      0x01
#define NBFI_UL_FLAG_UNENCRYPTED                0x02
#define NBFI_UL_FLAG_DEFAULT_PREAMBLE           0x04
#define NBFI_UL_FLAG_SEND_ON_CENTRAL_FREQ       0x08
#define NBFI_UL_FLAG_NO_RETRIES			0x10
#define NBFI_UL_FLAG_SAME_TX_ITER			0x20


typedef struct
{
  uint16_t id;
  nbfi_ul_status_t status;
  uint8_t reported;
  uint8_t flags;
}nbfi_ul_sent_status_t;



typedef struct
{
  uint16_t id;
  uint8_t length;
  uint8_t ready;
  #ifdef NBFI_USE_MALLOC
  uint8_t* payload;     //begining of packet payload
  #else
  uint8_t payload[NBFI_PACKET_SIZE*30];     //begining of packet payload
  #endif
}nbfi_dl_received_t;


//typedef void (*rx_handler_t)(uint8_t*, uint16_t);

#endif //NBFI_TYPES_H