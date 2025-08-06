/* See LICENSE file for license details */
/* pd - list the nics */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#ifdef __linux__
#include <linux/if_packet.h>
#else
#include <net/if_dl.h>
#endif

#define version "0.1"

void print_mac_addr(struct sockaddr *s){
	#ifdef __linux__
	struct sockaddr_ll *sl = (struct sockaddr_ll*)s;
	printf("%02x:%02x:%02x:%02x:%02x:%02x", sl->sll_addr[0], sl->sll_addr[1], sl->sll_addr[2], sl->sll_addr[3], sl->sll_addr[4], sl->sll_addr[5]);
	#else
	struct sockaddr_dl *sd = *(struct sockaddr_dl*)s;
	if(sd->sdl_alen == 6){
		unsigned char *p = (unsigned char*)LLADDR(sd);
		printf(("%02x:%02x:%02x:%02x:%02x:%02x", p[0], p[1], p[2], p[3], p[4], p[5]);
	}

	#endif
}

void print_if(struct ifaddrs *i){
	printf("%d: %s", if_nametoindex(i->ifa_name), i->ifa_name);
	printf(": [");
	if(i->ifa_flags & IFF_UP) printf("up");
	else printf("dw");
	if(i->ifa_flags & IFF_BROADCAST) printf(",bc");
	if(i->ifa_flags & IFF_RUNNING) printf(",rn");
	if(i->ifa_flags & IFF_MULTICAST) printf(",mc");
	if(i->ifa_flags & IFF_LOOPBACK) printf(",lb");
	printf("] ");
	printf("mt %d\n", 1500);
	if(i->ifa_addr){
		if(
			#ifdef __linux__
			i->ifa_addr->sa_family == AF_PACKET
			#else
			i->ifa_addr->sa_family == AF_LINK
			#endif
		){
			printf("    lk/");
			if(i->ifa_flags & IFF_LOOPBACK) printf("lb ");
			else printf("et ");
			print_mac_addr(i->ifa_addr);

			printf("\n");
		}
	}

	if(i->ifa_addr && i->ifa_addr->sa_family == AF_INET){
		struct sockaddr_in *ipv4 = (struct sockaddr_in*)i->ifa_addr;
		char ipstr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &ipv4->sin_addr, ipstr, INET_ADDRSTRLEN);
		printf("    inet %s", ipstr);
		if(i->ifa_netmask){
			struct sockaddr_in *m = (struct sockaddr_in*)i->ifa_netmask;
			inet_ntop(AF_INET, &m->sin_addr, ipstr, INET_ADDRSTRLEN);
			printf("/%s", ipstr);
		}

		printf("\n");
	}
}

void list_nics(){
	struct ifaddrs *i;
	struct ifaddrs *ii;
	char *sh_if[256] = {0};
	int cn = 0;
	if(getifaddrs(&i) == -1){
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	for(ii = i; ii != NULL; ii = ii->ifa_next){
		if(ii->ifa_addr == NULL) continue;
		int al_sh = 0;
		for(int i = 0; i < cn; i++){
			if(strcmp(sh_if[i], ii->ifa_name) == 0){
				al_sh = 1;
				break;
			}
		}

		if(!al_sh){
			print_if(ii);
			sh_if[cn++] = ii->ifa_name;
		}
	}

	freeifaddrs(i);
}

void show_version(){
	printf("pd-%s\n", version);
}

void help(){
	printf("usage: pd [options]..\n");
	printf("options:\n");
	printf("  -v	show version information\n");
	printf("  -h	display this\n");
}

int main(int argc, char *argv[]){
	if(argc == 2){
		if(strcmp(argv[1], "-h") == 0){
			help();
			return 0;
		} else if(strcmp(argv[1], "-v") == 0){
			show_version();
			return 0;
		}
	}

	list_nics();
	return 0;
}
