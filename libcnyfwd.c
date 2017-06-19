/*
 * This file is the confidential and proprietary property of Convey Computer.
 * Copyright (C) 2011-2012 Convey Computer Corporation. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "libcnyfwd.h"

#define DEBUG 0

static int mpfd;

int cny_fwd_open(uint32_t portNum)
{
    // Create a SOCKET
    mpfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mpfd < 0) 
	return -1;
    if (DEBUG) {
       printf("%s:successfully created socket\n",__FUNCTION__);
    }
    // Connect to the SOCKET
    struct sockaddr_in serv_addr; 
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(portNum);

    if (connect(mpfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
      if (DEBUG) {
        printf("%s:unable to connect to socket\n",__FUNCTION__);
      }
      return -1;
    }
    return 0;
}

int cny_fwd_close()
{
    return close(mpfd);
}

int cny_fwd_read(char *fpga, uint64_t off, uint64_t *data)
{
    int num;
    char msg[80];
    char str[3][32];
    uint64_t rtn_code,rtn_val;
    //we will send this command to the daemon 
    // "fpga rd aemc0 0x1000 1 0 !RTN_VAL:ULONG" 
    memset(msg, 0, sizeof(msg));
    sprintf (msg, "fpga rd %s 0x%lx 1 0 %s", fpga, off,EXPECT_ULONG_RTN );
    if (DEBUG) {
       printf("%s:debug %s",__FUNCTION__,msg);
    }
    num = send(mpfd, msg, sizeof(msg), 0);
    if (num != sizeof(msg))
	return -1;

    memset(msg, 0, sizeof(msg));
    num = recv(mpfd, msg, sizeof(msg), 0);
    if (num < 0)
	return -1;

    // "AE 0 0x1000: 0x0000000000000000\n"
    //PEEKPOKE will return this:"RTN:0x%x:VAL:0x%lx"
    num = sscanf(msg, "%[RTN:]%lx%[:]%[VAL:]%lx", str[0],&rtn_code,str[1],str[2],&rtn_val);
    if (num!=5 || rtn_code !=0) {
      if (DEBUG) {
         printf("  num = %d, rtn_code = %d, returned %s\n",num, (int) rtn_code, msg);
      }
      return -1;
    }

    *data=rtn_val;
    if (DEBUG) {
       printf(" returned <%s>\n",msg);
    }

    
    return 0;
}


int cny_fwd_write(char *fpga, uint64_t off, uint64_t data, uint64_t mask)
{
  //if mask is not zero, we do a read modify write using the mask
    int num;
    char snd[80], rcv[160];
    char str[9][32];
    uint64_t rtn_code;

    //if (ae < 0 || ae > 3)
    //	return -1;

    // "ae_csr_write ae 0 0x1000 0x0 0x0"
    memset(snd, 0, sizeof(snd));
    if (mask) {
      sprintf (snd, "fpga rmw %s  0x%lx 0x%lx 0x%lx %s ",
	       fpga, off, mask, data, EXPECT_NO_RTN );
      
    }
    else {
      sprintf (snd, "fpga wr %s  0x%lx 0x%lx  %s ",
	       fpga, off, data, EXPECT_NO_RTN );
    }

    if (DEBUG) {
       printf("%s:debug %s\n",__FUNCTION__,snd);
    }

    num = send(mpfd, snd, sizeof(snd), 0);
    if (num != sizeof(snd))
      return -1;

    memset(rcv, 0, sizeof(rcv));
    num = recv(mpfd, rcv, sizeof(rcv), 0);
    if (num < 0)
	return -1;

    // "Third parameter (offset) must be between 0x8000 and 0xffff\n" FIXME
    // "Wrote AE 0 0x1000 to 0x0000000000000000 with mask 0x0000000000000000\n"
    num = sscanf(rcv, "%[RTN:]%lx",
		 str[0], &rtn_code);
    if (num != 2 || rtn_code!=0)
      return -1;
    
    return 0;
}



int cny_fwd_cmd(char * command, ppc_rtn_val_t rtn_type, void * rtn_val_ptr, char * rslt_file_name)
{
  uint64_t rtn_val;
  int rtn_code;
  char msg[80];
  char str[9][32];
  char * msg_buff;
  int num;
  int err=0;
  char *start_of_rtn;

  if ((msg_buff=(char*) malloc(strlen(command)+40))==NULL) {
    if (DEBUG) {
      printf("%s:could not malloc buffer for command:\n\t%s\n",__FUNCTION__,command);
    }
    return -1;
  }
  strcpy(msg_buff,command);
  switch (rtn_type) {

  case UNSIGNED_LONG_T:
    strcat(msg_buff,EXPECT_ULONG_RTN);
    break;
    
  case STRING_T:
    strcat(msg_buff,EXPECT_ULONG_RTN);
    break;

  case RAW_SOCKET_T:
    strcat(msg_buff,EXPECT_RAW_SOCKET_RTN);
    break;
    
  case FILE_T:
    strcat(msg_buff,EXPECT_FILE_RTN);
    strcat(msg_buff,rslt_file_name);
    break;

  default:
    strcat(msg_buff,EXPECT_NO_RTN);
    break;
    
  }

  
  //send the command to the fwd
  num = send(mpfd, msg_buff, strlen(msg_buff), 0);
  if (num != strlen(msg_buff)) {
    err=1;
    goto LEAVE;
  }
  int done=0;
  do {
    
  //now get the response
    memset(msg, 0, sizeof(msg));
    num = recv(mpfd, msg, sizeof(msg),0);
    if (num < 0) {
      err=-1;
      goto LEAVE;
    }
    
    if((start_of_rtn=strcasestr(msg,"RTN:"))==0 )  {
      printf("%s", msg);
      continue;
    }
    else {
      done=1;
      switch (rtn_type) {
	
      case UNSIGNED_LONG_T:
	//fwd  will return this on the socket:"RTN:0x%x:VAL:0x%lx"
	
	num = sscanf(start_of_rtn, "%[RTN:]%x%[:]%[VAL:]%lx", str[0],&rtn_code,str[1],str[2],&rtn_val);
	if (num!=5 || rtn_code !=0) {
	  err= -1;
	}
	
	
	break;
	
#if(0)
      case STRING_T:
	//
	//fwd  will return this on the socket:"RTN:0x%x:VAL:{string}"
	num = sscanf(start_of_rtn, "%s[:]%x[:]%s[:]",str[0],str[1],&rtn_code,str[2],str[3],str[4]);
	if (num!=6 || rtn_code !=0) {
	  err= -1;
	}
	
	break;
#endif
	
	
      default:
	num = sscanf(start_of_rtn, "%[RTN:]%x", str[0],&rtn_code);
	if (num!=2 || rtn_code !=0) {
	  err= -1;
	}
	
	break;
	
      }
      if (rtn_code==COMMAND_NOT_FOUND) {
	//this is the command_not_found_returned from peekpoke_child.c in the fw daemon
	printf("%s:ERROR command not found!\n\t%s\n",__FUNCTION__,command);
      }
    }
  }while (!done);
    
  LEAVE: if(msg_buff!=NULL) free(msg_buff);
  return err;
}
