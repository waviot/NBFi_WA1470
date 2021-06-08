#include "stm32l0xx_hal.h"
#include "nbfi.h"
#include "time.h"
#include "buttons.h"
#include "gui.h"
#include "glcd.h"
#include "fonts.h"
//#include "adc.h"
//#include "main.h"
#include "nbfi_hal.h"



#define LCD_BACKLIGHT_Pin               GPIO_PIN_9
#define LCD_BACKLIGHT_GPIO_Port         GPIOA

#define LCD_BACKLIGHT_SWITCH_ON         HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET)
#define LCD_BACKLIGHT_SWITCH_OFF        HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET)

#define BUTTONS_RESP_TIME   30
#define BACKLIGHT_TIME      1000*10
#define ACTIVITY_TIME       1000*15



uint8_t last_rx_pkt[240];
uint8_t last_rx_pkt_len = 0;


static bool gui_update_state = false;
static uint32_t backlight_timer = 0;
static uint32_t gui_activity_timer = 0;

static bool gui_inited = false;

static volatile uint32_t gui_systimer = 0;

void GUI_systick()
{

  if((++gui_systimer%BUTTONS_RESP_TIME) == 0)
  {
    gui_update_state = true;
  }

  if(backlight_timer)
  {
    backlight_timer--;
    LCD_BACKLIGHT_SWITCH_ON;
  }
  else LCD_BACKLIGHT_SWITCH_OFF;

  if(gui_activity_timer) gui_activity_timer--;

}


void GUI_receive_complete(uint8_t * data, uint16_t length)
{
  for(uint8_t i = 0; i != length; i++) last_rx_pkt[i] = data[i];
  last_rx_pkt_len = length;
}

void (*current_handler)(void);

char textbuf[30]; // for formatted strings

void GUI_DrawButtonR(const char *label, uint8_t state);
void GUI_DrawButtonL(const char *label, uint8_t state);

void MainHandler();
void TestsHandler();
void NBFiTxHandler();
void NBFiRxHandler();
void RSSiHandler();
void InfoHandler();
void NBFiQuality();
void DevInfoHandler();
void NBFiStats();
void SettingsHandler();

//Text labels
const char label_enter[] =   "Enter";
const char label_back[] =    "Back";
const char label_start[] =   "Start";
const char label_stop[] =    "Stop";
const char label_cancel[] =  "Cancel";
const char label_edit[] =    "Edit";
const char label_dec[] =     "<";
const char label_inc[] =     ">";
const char label_reflash[] = "Reflash";

const gui_entry_t main_table[]=
{
  {"Tests",               &TestsHandler},
  {"Settings",            &SettingsHandler},
  {"Info",                &InfoHandler},
};

void MainHandler()
{
  static uint8_t state = 0;

  // Caption
  LCD_DrawString(0,(uint16_t)-6,"../", COLOR_FILL, ALIGN_LEFT);

  // Entry list
  for(int i=0; i<TABLE_SIZE(main_table); i++)
  {
    LCD_DrawString(20,(i*9)+5,main_table[i].label, COLOR_FILL, ALIGN_LEFT);
  }

  // Cursor
  LCD_DrawString(10,(state*9)+5,">", COLOR_FILL, ALIGN_LEFT);

  // Button processing
  if(GetButton1())
  {
    GUI_DrawButtonL(label_enter, 1);
    current_handler = main_table[state].handler;
  }
  else
  {
    GUI_DrawButtonL(label_enter, 0);
  }

  if(GetButton3())
  {
    state--;
    if(state > TABLE_SIZE(main_table))
      state = TABLE_SIZE(main_table)-1;
  }

  if(GetButton4())
  {
    state++;
    state = state % TABLE_SIZE(main_table);
  }
}

const gui_entry_t packet_tests_table[]=
{
  {"NBFi TX",            &NBFiTxHandler},
  {"NBFi RX",            &NBFiRxHandler},
  {"RSSi",            &RSSiHandler},
};

void TestsHandler()
{
  static uint8_t state_h = 0;

  // Caption
  LCD_DrawString(0,(uint16_t)-6,"../Tests", COLOR_FILL, ALIGN_LEFT);

  // Entry list
  for(int i=0; i<TABLE_SIZE(packet_tests_table); i++)
  {
    LCD_DrawString(20,(i*9)+5,packet_tests_table[i].label, COLOR_FILL, ALIGN_LEFT);
  }

  // Cursor
  LCD_DrawString(10,(state_h*9)+5,">", COLOR_FILL, ALIGN_LEFT);

  // Button processing
  if(GetButton1())
  {
    GUI_DrawButtonL(label_enter, 1);
    current_handler = packet_tests_table[state_h].handler;
  }
  else
  {
    GUI_DrawButtonL(label_enter, 0);
  }

  if(GetButton2())
  {
    GUI_DrawButtonR(label_back, 1);
    state_h = 0;
    current_handler = &MainHandler;
  }
  else
  {
    GUI_DrawButtonR(label_back, 0);
  }

  if(GetButton3())
  {
    state_h--;
    if(state_h > TABLE_SIZE(packet_tests_table))
      state_h = TABLE_SIZE(packet_tests_table)-1;
  }

  if(GetButton4())
  {
    state_h++;
    state_h = state_h % TABLE_SIZE(packet_tests_table);
  }
}

const char *gui_packet_state[]=
{
  "FREE",
  "ALLOCATED",
  "QUEUED",
  "TRANSMIT",
  "PROCESSING",
  "DL DELAY",
  "RECEIVE",
  "WAIT ACK",
  "GOT DL",
  "TIMEOUT"
};

extern uint8_t rx_pkt_len;
extern int16_t rssi;
extern int16_t offset;

extern struct axradio_callback_receive axradio_cb_receive;

const gui_entry_t nbfi_tx_table[]=
{
  {"Send short packet", 0},
  {"Send long packet",  0},
};

void NBFiTxHandler()
{
  static uint8_t state_n = 0;

  static uint8_t test_pkt[8] = {0,0xDE,0xAD,0xBE,0xEF,0x12,0x34,0x56};
  static uint8_t test_pkt_long[] = "Do you like this weather? I saw a politician with his hands in his own pockets.\n";

  // Caption
  LCD_DrawString(0,(uint16_t)-6,"../Tests/NBFi TX", COLOR_FILL, ALIGN_LEFT);

  // Entry list
  for(int i=0; i<TABLE_SIZE(nbfi_tx_table); i++)
  {
    LCD_DrawString(20,(i*9)+5,nbfi_tx_table[i].label, COLOR_FILL, ALIGN_LEFT);
  }

  // Cursor
  LCD_DrawString(10,(state_n*9)+5,">", COLOR_FILL, ALIGN_LEFT);

  // Button processing
  if(GetButton1())
  {
    GUI_DrawButtonL(label_enter, 1);
    switch(state_n)
    {
    case 0:
      NBFi_Send(test_pkt, sizeof(test_pkt));
      break;
    case 1:
      NBFi_Send(test_pkt_long, sizeof(test_pkt_long));
      break;
    }
    //current_handler = info_table[state_n].handler;
  }
  else
  {
    GUI_DrawButtonL(label_enter, 0);
  }

  if(GetButton2())
  {
    GUI_DrawButtonR(label_back, 1);
    state_n = 0;
    current_handler = &TestsHandler;
  }
  else
  {
    GUI_DrawButtonR(label_back, 0);
  }

  LCD_DrawString(10,36,"UL enqueued:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d", NBFi_Packets_To_Send());
  LCD_DrawString(127,36, textbuf, COLOR_FILL, ALIGN_RIGHT);

  if(GetButton3())
  {
    state_n--;
    if(state_n > TABLE_SIZE(nbfi_tx_table))
      state_n = TABLE_SIZE(nbfi_tx_table)-1;
  }

  if(GetButton4())
  {
    state_n++;
    state_n = state_n % TABLE_SIZE(nbfi_tx_table);
  }
}

void NBFiRxHandler()
{
  LCD_DrawString(0,(uint16_t)-6,"../Tests/NBFi RX", COLOR_FILL, ALIGN_LEFT);

  LCD_DrawString(2,5,"Last received HEX:", COLOR_FILL, ALIGN_LEFT);

  nbfi_state_t _nbfi_state;
  NBFi_get_state(&_nbfi_state);


  if(last_rx_pkt_len)
  {
    for(int i=0; i<last_rx_pkt_len; i++)//-1 for 1st CMD byte
    {
      if(2*i > 22) break;
      sprintf(textbuf+(2*i), "%02x", ((uint8_t*)last_rx_pkt)[i]); // +1 for 1st CMD byte
    }
    LCD_DrawString(2, 15, textbuf, COLOR_FILL, ALIGN_LEFT);
  }
  LCD_DrawString(2,25,"ASCII:", COLOR_FILL, ALIGN_LEFT);

  if(last_rx_pkt_len)
  {
    for(int i=0; i<last_rx_pkt_len; i++)//-1 for 1st CMD byte
    {
      if(i > 22) break;
      sprintf(textbuf+(i), "%c", ((uint8_t*)last_rx_pkt)[i]); // +1 for 1st CMD byte
    }

    LCD_DrawString(2, 35, textbuf, COLOR_FILL, ALIGN_LEFT);
  }

  sprintf(textbuf, "RSSI:%ddBm SNR:%d", _nbfi_state.last_rssi, _nbfi_state.last_snr);

  LCD_DrawString(2,45, textbuf, COLOR_FILL, ALIGN_LEFT);

  if(GetButton2())
  {
    current_handler = &TestsHandler;
  }

}


void RSSiHandler()
{

  static uint32_t last_update_time = 0;
  static float rssi;
  if((gui_systimer - last_update_time) > 250)
  {
    rssi = NBFi_get_rssi();
    last_update_time = gui_systimer;
  }

  nbfi_settings_t _nbfi;
  NBFi_get_Settings(&_nbfi);

  nbfi_state_t _nbfi_state;
  NBFi_get_state(&_nbfi_state);

  if(rssi) sprintf(textbuf, "RSSI  = %3.1f dBm", rssi);
  else sprintf(textbuf, "RSSI = ------ dBm");
  LCD_DrawString(10,5,textbuf, COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "NOISE = %3.1f dBm", NBFi_RF_get_noise());
  LCD_DrawString(10,15,textbuf, COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "FREQ = %u Hz", _nbfi_state.last_rx_freq);
  LCD_DrawString(10,25,textbuf, COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "BITRATE = %u",  NBFi_Phy_To_Bitrate(_nbfi.tx_phy_channel));
  LCD_DrawString(10,35,textbuf, COLOR_FILL, ALIGN_LEFT);

  // Caption
  LCD_DrawString(0,(uint16_t)-6,"../Tests/RSSi", COLOR_FILL, ALIGN_LEFT);

  // Button processing
  if(GetButton2())
  {
      GUI_DrawButtonR(label_back, 1);
      current_handler = &TestsHandler;
  }
  else
  {
      GUI_DrawButtonR(label_back, 0);
  }
}


const char fmt_uint8_t[] = "%u";
const char fmt_int8_t[] = "%d";
const char fmt_uint32_t[] = "%d";
const char fmt_str[] = "%s";

void SettingsHandler()
{
  static int8_t state_s = 0;
  static uint8_t edit = 0;

  #define TOTAL_NUMBER_OF_SETTINGS      6

  static uint8_t scroll = 0;


  // Caption
  LCD_DrawString(0,(uint16_t)-6,"../Settings", COLOR_FILL, ALIGN_LEFT);

  static nbfi_settings_t _nbfi = {0};

  if(!_nbfi.modem_id) NBFi_get_Settings(&_nbfi);

  // Entry list
  for(int i = scroll; i < scroll + GUI_NUM_LINES; i++)                                            //for(int i=0; i<4 && i<GUI_NUM_LINES; i++)
  {
    switch(i)
    {
    case 0:
      LCD_DrawString(10,((i-scroll)*9)+5, " BitRates", COLOR_FILL, ALIGN_LEFT);
      if(_nbfi.additional_flags&NBFI_FLG_FIXED_BAUD_RATE) sprintf(textbuf, "%d", NBFi_Phy_To_Bitrate(_nbfi.tx_phy_channel));
      else sprintf(textbuf, "AUTO");
      LCD_DrawString(127,((i-scroll)*9)+5, textbuf, COLOR_FILL, ALIGN_RIGHT);
      break;
    case 1:
      LCD_DrawString(10,((i-scroll)*9)+5, " TX Antenna", COLOR_FILL, ALIGN_LEFT);
      sprintf(textbuf, "%s", _nbfi.tx_antenna?"SMA":"PCB");
      LCD_DrawString(127,((i-scroll)*9)+5, textbuf, COLOR_FILL, ALIGN_RIGHT);
      break;
    case 2:
      LCD_DrawString(10,((i-scroll)*9)+5, " RX Antenna", COLOR_FILL, ALIGN_LEFT);
      sprintf(textbuf, "%s", _nbfi.rx_antenna?"SMA":"PCB");
      LCD_DrawString(127,((i-scroll)*9)+5, textbuf, COLOR_FILL, ALIGN_RIGHT);
      break;
    case 3:
      LCD_DrawString(10,((i-scroll)*9)+5, " NBFi Mode", COLOR_FILL, ALIGN_LEFT);
      switch(_nbfi.mode)
      {
        case NRX:
          sprintf(textbuf, "%s", "NRX");
          break;
        case DRX:
          sprintf(textbuf, "%s", "DRX");
          break;
        case CRX:
          sprintf(textbuf, "%s", "CRX");
          break;
        default:
          sprintf(textbuf, "%s", "OFF");
          break;

      }
      LCD_DrawString(127,((i-scroll)*9)+5, textbuf, COLOR_FILL, ALIGN_RIGHT);
      break;
    case 4:
      LCD_DrawString(10,((i-scroll)*9)+5, " HB Interval, min", COLOR_FILL, ALIGN_LEFT);
      if(_nbfi.heartbeat_num == 255)  sprintf(textbuf, "%u", (_nbfi.mode == CRX)?_nbfi.heartbeat_interval/60:_nbfi.heartbeat_interval);
      else sprintf(textbuf, "OFF");
      LCD_DrawString(127,((i-scroll)*9)+5, textbuf, COLOR_FILL, ALIGN_RIGHT);
      break;
    case 5:
      LCD_DrawString(10,((i-scroll)*9)+5, " Base Freqs", COLOR_FILL, ALIGN_LEFT);
      if(_nbfi.ul_freq_base == 868800000) sprintf(textbuf, "RU");
      else if(_nbfi.ul_freq_base == 868100000) sprintf(textbuf, "EU");
      else if(_nbfi.ul_freq_base == 866900000) sprintf(textbuf, "IN");
      else if(_nbfi.ul_freq_base == 864000000) sprintf(textbuf, "KZ");
      else if(_nbfi.ul_freq_base == 458550000) sprintf(textbuf, "UZ");
      else if(_nbfi.ul_freq_base == 916500000) sprintf(textbuf, "ARG");
      else sprintf(textbuf, "UNKN");
      LCD_DrawString(127,((i-scroll)*9)+5, textbuf, COLOR_FILL, ALIGN_RIGHT);
      break;
    }
  }

  // Button processing
  if(edit)
  {
    LCD_FillRect(0,((state_s-scroll)*9)+10,128,10, COLOR_INVERT);

    if(GetButton1())
    {
      GUI_DrawButtonL(label_inc, 1);
      switch(state_s)
      {
      case 0:
        if(_nbfi.additional_flags&NBFI_FLG_FIXED_BAUD_RATE)
        {
          switch(_nbfi.tx_phy_channel)
          {
            case UL_DBPSK_50_PROT_E:
              _nbfi.tx_phy_channel = UL_DBPSK_400_PROT_E;
              _nbfi.rx_phy_channel = DL_DBPSK_400_PROT_D;
              break;
            case UL_DBPSK_400_PROT_E:
              _nbfi.tx_phy_channel = UL_DBPSK_3200_PROT_E;
              _nbfi.rx_phy_channel = DL_DBPSK_3200_PROT_D;
              break;
            case UL_DBPSK_3200_PROT_E:
              _nbfi.tx_phy_channel = UL_DBPSK_25600_PROT_E;
              _nbfi.rx_phy_channel = DL_DBPSK_25600_PROT_D;
              break;
            case UL_DBPSK_25600_PROT_E:
              _nbfi.tx_phy_channel = UL_DBPSK_50_PROT_E;
              _nbfi.rx_phy_channel = DL_DBPSK_50_PROT_D;
              _nbfi.additional_flags &= ~NBFI_FLG_FIXED_BAUD_RATE;
              break;
          }
        }
        else
        {
              _nbfi.tx_phy_channel = UL_DBPSK_50_PROT_E;
              _nbfi.rx_phy_channel = DL_DBPSK_50_PROT_D;
              _nbfi.additional_flags |= NBFI_FLG_FIXED_BAUD_RATE;

        }

        break;
      case 1:
        _nbfi.tx_antenna ^= 1;
        break;
      case 2:
        _nbfi.rx_antenna ^= 1;
        break;
      case 3:
        _nbfi.mode++;
        if(_nbfi.mode > OFF) _nbfi.mode = NRX;
        if( _nbfi.mode > CRX) _nbfi.mode = OFF;
        if(_nbfi.mode == CRX && _nbfi.heartbeat_num) _nbfi.heartbeat_interval *= 60;
        else if(_nbfi.mode == NRX && _nbfi.heartbeat_num) _nbfi.heartbeat_interval /= 60;
        break;
      case 4:
        if(_nbfi.mode == CRX)
        {
          if(_nbfi.heartbeat_interval >= 60*10) {_nbfi.heartbeat_interval = 0;_nbfi.heartbeat_num = 0;}
          else {_nbfi.heartbeat_interval += 60; _nbfi.heartbeat_num = 255;}
        }
        else
        {
          if(_nbfi.heartbeat_interval >= 10) {_nbfi.heartbeat_interval = 0;_nbfi.heartbeat_num = 0;}
          else {_nbfi.heartbeat_interval++; _nbfi.heartbeat_num = 255;}
        }

        break;
      case 5:
        switch(_nbfi.ul_freq_base)
          {
            case 868800000:
              _nbfi.ul_freq_base = 868100000;
              _nbfi.dl_freq_base = 869500000;
              break;
            case 868100000:
              _nbfi.ul_freq_base = 866900000;
              _nbfi.dl_freq_base = 865100000;
              break;
            case 866900000:
              _nbfi.ul_freq_base = 864000000;
              _nbfi.dl_freq_base = 863500000;
              break;
            case 864000000:
              _nbfi.ul_freq_base = 458550000;
              _nbfi.dl_freq_base = 453800000;
              break;
            case 458550000:
              _nbfi.ul_freq_base = 916500000;
              _nbfi.dl_freq_base = 903000000;
              break;
            case 916500000:
              _nbfi.ul_freq_base = 868800000;
              _nbfi.dl_freq_base = 869150000;
              break;
          }
        break;
      }

    }
    else
    {
      GUI_DrawButtonL(label_inc, 0);
    }

    if(GetButton2())
    {
      GUI_DrawButtonR(label_back, 1);
      edit = 0;
      NBFi_set_Settings(&_nbfi, 1);
    }
    else
    {
      GUI_DrawButtonR(label_back, 0);
    }
  }
  else
  {
    // Cursor
    LCD_DrawString(5,((state_s-scroll)*9)+5,">", COLOR_FILL, ALIGN_LEFT);

    NBFi_get_Settings(&_nbfi);


    if(GetButton1())
    {
      GUI_DrawButtonL(label_edit, 1);
      edit = 1;
    }
    else
    {
      GUI_DrawButtonL(label_edit, 0);
    }

    if(GetButton2())
    {
      GUI_DrawButtonR(label_back, 1);
      state_s = 0;
      scroll = 0;
      current_handler = &MainHandler;
    }
    else
    {
      GUI_DrawButtonR(label_back, 0);
    }

    if(GetButton3())
    {
      if(--state_s < 0 ) state_s = 0;

      if(state_s - scroll < 0) scroll--;

    }

    if(GetButton4())
    {

      if(++state_s >= TOTAL_NUMBER_OF_SETTINGS)
      {

        state_s = TOTAL_NUMBER_OF_SETTINGS - 1;

      }

      if(state_s - GUI_NUM_LINES + 1 > 0 ) scroll = state_s - GUI_NUM_LINES + 1;

    }
  }
}


const gui_entry_t info_table[]=
{
  {"NBFi quality",      &NBFiQuality},
  {"NBFi statistics",   &NBFiStats},
  {"Device",            &DevInfoHandler},
};

void InfoHandler()
{
  static uint8_t state_i = 0;

  LCD_DrawString(0,(uint16_t)-6,"../Info", COLOR_FILL, ALIGN_LEFT);

  // Entry list
  for(int i=0; i<TABLE_SIZE(info_table); i++)
  {
    LCD_DrawString(20,(i*9)+5,info_table[i].label, COLOR_FILL, ALIGN_LEFT);
  }

  // Cursor
  LCD_DrawString(10,(state_i*9)+5,">", COLOR_FILL, ALIGN_LEFT);

  // Button processing
  if(GetButton1())
  {
    GUI_DrawButtonL(label_enter, 1);
    current_handler = info_table[state_i].handler;
  }
  else
  {
    GUI_DrawButtonL(label_enter, 0);
  }

  if(GetButton2())
  {
    GUI_DrawButtonR(label_back, 1);
    state_i = 0;
    current_handler = &MainHandler;
  }
  else
  {
    GUI_DrawButtonR(label_back, 0);
  }

  if(GetButton3())
  {
    state_i--;
    if(state_i > TABLE_SIZE(info_table))
      state_i = TABLE_SIZE(info_table)-1;
  }

  if(GetButton4())
  {
    state_i++;
    state_i = state_i % TABLE_SIZE(info_table);
  }
}

void NBFiQuality()
{

  nbfi_settings_t _nbfi;
  NBFi_get_Settings(&_nbfi);

  nbfi_state_t _nbfi_state;
  NBFi_get_state(&_nbfi_state);


  LCD_DrawString(0,(uint16_t)-6,"../Info/NBFi quality", COLOR_FILL, ALIGN_LEFT);

  LCD_DrawString(10,5,"Noise lev:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%3.1f dBm", NBFi_RF_get_noise());
  LCD_DrawString(127,5, textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,15,"UL aver. SNR:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d", _nbfi_state.aver_tx_snr);
  LCD_DrawString(127,15, textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,25,"DL aver. SNR:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d", _nbfi_state.aver_rx_snr);
  LCD_DrawString(127,25, textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,35,"UL bitrate:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d", NBFi_Phy_To_Bitrate(_nbfi.tx_phy_channel));
  LCD_DrawString(127,35, textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,45,"DL bitrate:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%u", NBFi_Phy_To_Bitrate(_nbfi.rx_phy_channel));
  LCD_DrawString(127,45, textbuf, COLOR_FILL, ALIGN_RIGHT);

  if(GetButton2())
  {
    current_handler = &InfoHandler;
  }
}

void NBFiStats()
{

  LCD_DrawString(0,(uint16_t)-6,"../Info/NBFi statistics", COLOR_FILL, ALIGN_LEFT);

  nbfi_state_t _nbfi_state;
  NBFi_get_state(&_nbfi_state);

  LCD_DrawString(10,5,"UL enqueued:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d", NBFi_Packets_To_Send());
  LCD_DrawString(127,5, textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,15,"UL delivered:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d", _nbfi_state.success_total);
  LCD_DrawString(127,15, textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,25,"UL lost:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d", _nbfi_state.fault_total);
  LCD_DrawString(127,25, textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,35,"UL total:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d", _nbfi_state.UL_total);
  LCD_DrawString(127,35, textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,45,"DL total:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d", _nbfi_state.DL_total);
  LCD_DrawString(127,45, textbuf, COLOR_FILL, ALIGN_RIGHT);

  if(GetButton2())
  {
    current_handler = &InfoHandler;
  }
}

void DevInfoHandler()
{
  LCD_DrawString(0,(uint16_t)-6,"../Info/Device info", COLOR_FILL, ALIGN_LEFT);


  nbfi_settings_t _nbfi;
  NBFi_get_Settings(&_nbfi);

  // Device full ID
  LCD_DrawString(10,5,"ID:", COLOR_FILL, ALIGN_LEFT);

  sprintf(textbuf, "%d", *_nbfi.modem_id);
  LCD_DrawString(127,5,textbuf, COLOR_FILL, ALIGN_RIGHT);

  time_t t = NBFi_get_RTC();
  struct tm *TM = localtime(&t);

  LCD_DrawString(10,15,"Time:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%02d:%02d:%02d", TM->tm_hour, TM->tm_min, TM->tm_sec);
  LCD_DrawString(127,15,textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,25,"VCC:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%.2fV", (float)nbfi_HAL_measure_valtage_or_temperature(1)/100);
  LCD_DrawString(127,25,textbuf, COLOR_FILL, ALIGN_RIGHT);

  LCD_DrawString(10,35,"TEMP:", COLOR_FILL, ALIGN_LEFT);
  sprintf(textbuf, "%d'C", nbfi_HAL_measure_valtage_or_temperature(0));
  LCD_DrawString(127,35,textbuf, COLOR_FILL, ALIGN_RIGHT);

  if(GetButton2())
  {
    GUI_DrawButtonR(label_back, 1);
    current_handler = &InfoHandler;
  }
  else
  {
    GUI_DrawButtonR(label_back, 0);
  }
}

//-------------------------------- SERVICE ROUTINES -------------------------------------

void GUI_Update()
{
  if(!gui_update_state) return;
  gui_update_state = false;

  LCD_ClearBuffer(0);
  LCD_SetFont(&arial_8ptFontInfo);

  (*current_handler)();

  LCD_WriteBuffer();
}

void GUI_Init()
{
  LCD_Init();

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin = LCD_BACKLIGHT_Pin;
  HAL_GPIO_Init(LCD_BACKLIGHT_GPIO_Port, &GPIO_InitStruct);
  current_handler = &MainHandler;
  gui_inited = true;

}

void GUI_Deinit()
{
  LCD_BACKLIGHT_SWITCH_OFF;
  LCD_Deinit();
  gui_inited = false;
}

bool GUI_is_inited()
{
  return gui_inited;
}

void GUI_DrawButtonL(const char *label, uint8_t state)
{
  if(state)
  {
    LCD_FillRect(0, 52, 51, 12, 1);
    LCD_DrawString(26,48,label, COLOR_INVERT, ALIGN_CENTERED);
  }
  else
  {
    LCD_DrawRect(0, 52, 51, 12, 1);
    LCD_DrawString(26,48,label, COLOR_FILL, ALIGN_CENTERED);
  }
}

void GUI_DrawButtonR(const char *label, uint8_t state)
{
  if(state)
  {
    LCD_FillRect(76, 52, 51, 12, 1);
    LCD_DrawString(102,48,label, COLOR_INVERT, ALIGN_CENTERED);
  }
  else
  {
    LCD_DrawRect(76, 52, 51, 12, 1);
    LCD_DrawString(102,48,label, COLOR_FILL, ALIGN_CENTERED);
  }
}

void GUI_extend_activity()
{
  backlight_timer = BACKLIGHT_TIME;
  gui_activity_timer = ACTIVITY_TIME;
}

bool GUI_can_sleep()
{
  return (backlight_timer == 0)&&(gui_activity_timer == 0);
}