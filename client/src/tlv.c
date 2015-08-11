#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <limits.h>
#include <string.h>
#include "common.h" 
#include "tlv.h"

#define DEBUG 1

int encode_string_data(const char *data, const int length, Buffer *buf)
{
   char *curP = buf->payload + buf->length;
   
   if ((length <= 0) || (length >= 1024))
   {
     printf(" %s :  can't encode , Data Length = %d", __FUNCTION__, length);
     return 0;
   }

   *(uint16_t *)curP = htons(STRING_DATA);
   *(uint16_t *)(curP + 2) = htons(length);
    buf->length  += 4;
    memcpy(curP + 4 , data, length);
    buf->length  += length;

#if (DEBUG)   
    printf("Encoded Length = %d \n", buf->length);
    printf("%s ...%s\n", __FUNCTION__, buf->payload);
#endif   
 
  return buf->length;
}

int encode_join_group(uint32_t *groups, int numgroups , Buffer *buf)
{
   char *curP = buf->payload + buf->length;
   
   if ((numgroups <= 0) || (numgroups >= 255))
   {
    printf(" %s :  can't encode , Data Length = %d", __FUNCTION__, numgroups);
    return 0;
   }
   
   *(uint16_t *)curP = htons(JOIN_GROUP);
   *(uint16_t *)(curP + 2) = htons(4*numgroups);
    curP = curP + 4;
    buf->length  += 4;
    for (int i = 0; i < numgroups; i++)
    {
     *(uint32_t *)curP = htons(groups[i]);
      curP = curP + 4;
      buf->length  += 4;
    } 

#if (DEBUG)   
    printf("Encoded Length = %d \n", buf->length);
    printf("%s ...%s\n", __FUNCTION__, buf->payload);
#endif
  return buf->length;
}

int encode(Attribute attr, const void *data, const int length, Buffer *buf)
{
  switch(attr)
  {
   case JOIN_GROUP:
    printf("Encoding JOIN_GROUP\n");
    return encode_join_group((uint32_t*)data, length, buf);
   case STRING_DATA:
    printf("Encoding STRING_DATA \n");
    return encode_string_data(data, length, buf);
   default:
    printf("%s : can't Understand the Attribute to be encoded", __FUNCTION__);
    break;
  }
  return 0;
}
Tlv_element decode(char *buffer, unsigned int buflen)
{
  printf("Buflen = %d %s \n", buflen, buffer);
  Tlv_element  tlv;
  
  tlv.type = htons(*(uint16_t*)buffer);
  tlv.length = htons(*(uint16_t *)(buffer + 2));
  buflen = buflen - 4;
  buffer = buffer + 4;
  switch(tlv.type)
  {
    case STRING_DATA:
     tlv.value = buffer;
     //buflen  = buflen - tlv.length;
     //memcpy(tlv.value, buffer + 4, tlv.length);
     //printf(" Buffer Length %d and Value %s",buflen, tlv.value);
     break;
    case JOIN_GROUP:
    {
     int numgroups = tlv.length/4;
     int groups[255];
     printf("Num Groups = %d", numgroups);
     for (int i = 0 ; i < numgroups ; i++)
     {
       groups[i] = htons(*(uint32_t *)buffer);
       buffer = buffer + 4;
       printf(" Joining Group : %d \n", groups[i]);
     }
    }
    break;
    default:
     printf(" %s Can't Decode %d ! ", __FUNCTION__,htons(*(uint16_t*)buffer) ); 
     break;   
  }
   return tlv;
}




