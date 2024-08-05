#include <net/if.h>
#include <net/if_tap.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>

int main()
{
   int fd0 = open("/dev/tap0", O_RDWR);
   if (fd0 == -1) {
      printf("Error: open(/dev/tap0) failed\n");
      return 1;
   }

   char mac[6];
   memset(mac, 0, sizeof(mac));
   if (ioctl(fd0, SIOCGIFADDR, (void *)mac) < 0) {
      printf("Error: Could not get MAC address of /dev/tap0.\n");
      close(fd0);
      return 1;
   }

   printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2],
                                                  mac[3], mac[4], mac[5]);

   enum { BUFFLEN = 1500 };
   char buffer[BUFFLEN];
   while (1) {
      ssize_t received = read(fd0, buffer, BUFFLEN);
      if (received < 0) {
         close(fd0);
         return 1;
      }

      printf("Received packet with %d bytes\n", received);
      size_t i=0;
      uint32_t *words = (uint32_t*)buffer;
      for (; i < received / 4; i++) {
         printf("%08x ", *words++);

         if (i % 4 == 3)
            printf("\n");
      }

      uint8_t *bytes = (uint8_t*)&buffer[i*4];
      for (i*=4; i < received; i++) {
         printf("%02x", *bytes++);
      }
      printf("\n");
   }
}
