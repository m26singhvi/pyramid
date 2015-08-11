#ifndef __TLV_H__
#define __TLV_H__

#include <stdint.h>

#define BUFFER_SIZE 1024

typedef enum Attribute
{
  NONE = 0,
  JOIN_GROUP = 1,
  GOOD_BYE = 2,
  STRING_DATA = 3,
  INT32_DATA = 4,
  CLI_DATA = 5, 
  MAX = 65536
}Attribute;

typedef struct 
{
  int length;
  char *payload;
}Buffer;

typedef struct
{
  uint16_t type;
  uint16_t length;
  char *value;
}Tlv_element;

int encode(Attribute attr, const void *data, const int length, Buffer *buf);
int encode_string_data(const char *data, const int length, Buffer *buf);
int encode_cli_data(const char *data, const int length, Buffer *buf);
int encode_join_group(uint32_t *data, const int length, Buffer *buf);
Tlv_element decode(char *buffer, unsigned int buflen);


#endif /* __TLV_H__ */
