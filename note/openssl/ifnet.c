#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	struct sockaddr_in *bc;

	getifaddrs(&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next)
	{

		if (ifa->ifa_addr->sa_family == AF_INET)
		{

			sa = (struct sockaddr_in *)ifa->ifa_addr;
			bc = (struct sockaddr_in *)ifa->ifa_broadaddr; /* ifa->ifa_dstaddr; */

			printf("name: %s\n", ifa->ifa_name);
			printf("\tinet: %s\n", inet_ntoa(sa->sin_addr));
			printf("\tbroadcast: %s\n", inet_ntoa(bc->sin_addr));

			if ((strncmp(ifa->ifa_name, "en0", 3) == 0) ||
				(strncmp(ifa->ifa_name, "en1", 4) == 0) ||
				(strncmp(ifa->ifa_name, "em0", 3) == 0) ||
				(strncmp(ifa->ifa_name, "eth0", 4) == 0))
			{
				printf("inet: %s\n", inet_ntoa(sa->sin_addr));
				printf("broadcast: %s\n", inet_ntoa(bc->sin_addr));

				freeifaddrs(ifap);
				return 0;
			}
		}
	}

	freeifaddrs(ifap);
	return 0;
}