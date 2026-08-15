/********************************** (C) COPYRIGHT *********************************
 * File Name          : https.h
 * Author             : WCH
 * Version            : V2.0
 * Date               : 2025/04/02
 * Description        : web API
 *
 **********************************************************************************/
#ifndef __HTTPS_H__
#define __HTTPS_H__
#include "debug.h"

#define HTTP_SERVER_PORT 80

#define IPCHANGE 0x01
#define PORTCHANGE 0x02
#define RECVDATA 0x04
#define SENDDATA 0x08

/* HTTP Method */
#define METHOD_ERR 0  /**< Error Method. */
#define METHOD_GET 1  /**< GET Method.   */
#define METHOD_HEAD 2 /**< HEAD Method.  */
#define METHOD_POST 3 /**< POST Method.  */

/* HTTP GET Method */
#define PTYPE_ERR 0	  /**< Error file. */
#define PTYPE_HTML 1  /**< HTML file.  */
#define PTYPE_GIF 2	  /**< GIF file.   */
#define PTYPE_TEXT 3  /**< TEXT file.  */
#define PTYPE_JPEG 4  /**< JPEG file.  */
#define PTYPE_FLASH 5 /**< FLASH file. */
#define PTYPE_MPEG 6  /**< MPEG file.  */
#define PTYPE_PDF 7	  /**< PDF file.   */
#define PTYPE_CGI 8	  /**< CGI         */

/* HTML Doc. for ERROR */
#define ERROR_HTML_PAGE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\r\n</BODY>\r\n</HTML>\r\n\0"
// static char  ERROR_HTML_PAGE[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\r\n</BODY>\r\n</HTML>\r\n\0";

#define ERROR_REQUEST_PAGE "HTTP/1.1 400 OK\r\nContent-Type: text/html\r\nContent-Length: 50\r\n\r\n<HTML>\r\n<BODY>\r\nInvalid request.\r\n</BODY>\r\n</HTML>\r\n\0"
// static char ERROR_REQUEST_PAGE[] = "HTTP/1.1 400 OK\r\nContent-Type: text/html\r\nContent-Length: 50\r\n\r\n<HTML>\r\n<BODY>\r\nInvalid request.\r\n</BODY>\r\n</HTML>\r\n\0";

#define RETURN_CGI_PAGE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "

/* Response header for HTML*/
#define RES_HTMLHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
// static PROGMEM char RES_HTMLHEAD_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
/* Response head for TEXT */
#define RES_TEXTHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "

/* Response head for GIF */
#define RES_GIFHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Length: "

/* Response head for JPEG */
#define RES_JPEGHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: "

/* Response head for FLASH */
#define RES_FLASHHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/x-shockwave-flash\r\nContent-Length: "

/* Response head for MPEG */
#define RES_MPEGHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: video/mpeg\r\nContent-Length: "

/* Response head for PDF */
#define RES_PDFHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Length: "

/*-----------------*/

#define TX_RX_MAX_BUF_SIZE 2048
#define MAX_URI_SIZE 128

#define TX_BUF tx_buf
#define RX_BUF rx_buf

typedef struct _st_http_request
{
	UINT8 METHOD;
	UINT8 TYPE;
	UINT8 URI[MAX_URI_SIZE];
} st_http_request;

#define WEB_SERVER_STEP_IDLE 0		  // Idle state
#define WEB_SERVER_STEP_PROCESSING 1  // Processing request
#define WEB_SERVER_STEP_SEND_HEADER 2 // Sending response header
#define WEB_SERVER_STEP_SEND_DATA 3	  // Sending data
#define WEB_SERVER_STEP_SEND_SUC 4	  // Sending data

void ParseHttpRequest(st_http_request *, UINT8 *);

void ParseUriType(UINT8 *, char *);
void MakeHttpResponse(unsigned char *, char);
unsigned char *GetUriName(char *uri);
void GetAddr(unsigned char *addr, unsigned char *ip);
char *DataLocate(char *buf, char *name);
void LedControl(UINT8 *buf);
void IpSeting(UINT8 *buf1, UINT8 *buf2);
void IpRefrsh(UINT8 *buf, UINT8 *buf2);
UINT16 PortSeting(UINT8 *buf);
void PortRefresh(UINT8 *buf, UINT16 port);
void TempratureRefresh(UINT8 *buf, UINT8 temp);
void CurrentRefresh(UINT8 *buf, UINT8 curr);
void VoltageRefresh(UINT8 *buf, UINT8 volt);
void ADCData(UINT8 *buf); /* Random number generation, simulating ADC results */
void WebServer(void);

#endif
