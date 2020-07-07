#ifndef NBFI_AT_SERVER_TAGS_H
#define NBFI_AT_SEVER_TAGS_H


#define NBFI_AT_SERVER_TAGS_NUMBER   42  
#define NBFI_AT_SERVER_TAGS_MAX_LEN  20   

extern const char nbfi_at_server_tags_mas[NBFI_AT_SERVER_TAGS_NUMBER][NBFI_AT_SERVER_TAGS_MAX_LEN];


nbfi_at_server_tags_t nbfi_at_server_str2tag(const char *str);
_Bool nbfi_at_server_tag2str(nbfi_at_server_tags_t, char *str);
uint8_t* nbfi_at_server_get_sub_param(uint8_t *param);

#endif //NBFI_AT_SEVER_TAGS_H